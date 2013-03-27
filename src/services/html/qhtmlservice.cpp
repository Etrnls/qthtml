/****************************************************************************
**
** Copyright (C) 2011 by Etrnls
** etrnls@gmail.com
**
** This file is part of the Qt HTML service daemon.
**
** Qt HTML service daemon is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation, either version 3 of the License,
** or (at your option) any later version.
**
** Qt HTML service daemon is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Qt HTML service daemon. If not, see
** <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "qhtmlservice.h"

#include <QtCore/QtEndian>
#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QCryptographicHash>
#include <QtNetwork/QTcpSocket>
#include <QtGui/QImage>
#include <QtCore/QtDebug>
#include <QtCore/QMutexLocker>

QT_BEGIN_NAMESPACE

QHtmlService::QHtmlService() : mScreenGeometry(0, 0, INT_MAX, INT_MAX)
{
    mDebug = true;
    connect(&mServer, SIGNAL(newConnection()), SLOT(onServerNewConnection()));
    mServer.listen(QHostAddress::Any, 8080); // TODO configurable address and port
}

int QHtmlService::getScreenWidth() const
{
    return mScreenGeometry.width();
}

int QHtmlService::getScreenHeight() const
{
    return mScreenGeometry.height();
}

void QHtmlService::changeCursor(int shape) const
{
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::ChangeCursor))), shape);
}

int QHtmlService::allocateWinId(int windowType)
{
    int newWinId = 1;
    {
    QMutexLocker lock(&mWinIdsMutex);
    foreach (int winId, mWinIds)
        if (newWinId > winId)
            break;
        else
            ++newWinId;
    mWinIds.insert(newWinId);
    }

    const int popup = windowType == Qt::Popup ? 1 : 0;
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::CreateWindow))), newWinId, popup);
    return newWinId;
}

void QHtmlService::deallocateWinId(int winId)
{
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::DestroyWindow))), winId);
    QMutexLocker lock(&mWinIdsMutex);
    mWinIds.remove(winId);
}

void QHtmlService::setGeometry(int winId, int x, int y, int width, int height) const
{
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::SetGeometry))), winId, x, y, width, height);
}

void QHtmlService::setVisible(int winId, bool visible) const
{
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::SetVisible))), winId, visible);
}

void QHtmlService::setWindowTitle(int winId, const QString &title) const
{
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::SetWindowTitle))), winId, QString::fromLatin1(title.toUtf8().toBase64().constData()));
}

void QHtmlService::raise(int winId) const
{
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::Raise))), winId);
}

void QHtmlService::flush(int winId, int x, int y, int width, int height, const QByteArray &imageData) const
{
    const QString url = QStringLiteral("data:image/png;base64,") + QString::fromLatin1(imageData.toBase64().constData());
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::Flush))), winId, x, y, width, height, url);
}

void QHtmlService::scroll(int winId, int x, int y, int width, int height, int dx, int dy) const
{
    sendMessage(QString(QChar::fromLatin1(static_cast<char>(MessageCommand::Scroll))), winId, x, y, width, height, dx, dy);
}

void QHtmlService::onServerNewConnection()
{
    while (mServer.hasPendingConnections()) {
        QTcpSocket * const socket = mServer.nextPendingConnection();
        //if (mDebug)
        //     qDebug() << "mServer.nextPendingConnection";
        connect(socket, SIGNAL(readyRead()), SLOT(onHttpSocketReadyRead()));
        connect(socket, SIGNAL(disconnected()), SLOT(onSocketDisconnected()));
    }
}

void QHtmlService::onHttpSocketReadyRead()
{
    QTcpSocket * const socket = qobject_cast<QTcpSocket*>(sender());
    Q_ASSERT(socket);

    //QMutexLocker lock(&mWebSocketHandshakeHeadersMutex);

    // If we are waiting for full websocket handshake header
    QHash<QTcpSocket *, QStringList>::iterator header = mWebSocketHandshakeHeaders.find(socket);
    if (header != mWebSocketHandshakeHeaders.end()) {
        while (socket->canReadLine()) {
            const QString line = QString::fromLatin1(socket->readLine().constData());
            if (line == QStringLiteral("\r\n")) {
                disconnect(socket, SIGNAL(readyRead()), this, SLOT(onHttpSocketReadyRead()));
                sendWebSocketHandshake(socket);
                return;
            } else {
                header->append(line);
            }
        }
    }

    // handle http request
    if (socket->canReadLine()) {
        static const QRegExp requestRegExp(QStringLiteral("GET (.*) HTTP/1.1\r\n"));
        const QString request = QString::fromLatin1(socket->readLine().constData());

        if (requestRegExp.exactMatch(request)) {
            QString location = requestRegExp.cap(1);

            if (location == QStringLiteral("/socket")) {
                mWebSocketHandshakeHeaders.insert(socket, QStringList());
                onHttpSocketReadyRead();
            } else {
                if (location == QStringLiteral("/"))
                    location = QStringLiteral("/client.html");

                QFile file(QStringLiteral(":") + location);
                if (file.exists()) {
                    file.open(QFile::ReadOnly);
                    sendStaticResponse(socket, 200, guessMimeType(QFileInfo(file).suffix()), file.readAll());
                } else {
                    sendStaticResponse(socket, 404);
                }
            }
        } else {
            sendStaticResponse(socket, 501);
        }
    }
}

void QHtmlService::onWebSocketReadyRead()
{
    QTcpSocket * const socket = qobject_cast<QTcpSocket *>(sender());
    QByteArray arr;
    {
    QMutexLocker lock(&mWebSocketFrameBuffersMutex);
    QHash<QTcpSocket *, QByteArray>::Iterator iter = mWebSocketFrameBuffers.find(socket);
    if (iter == mWebSocketFrameBuffers.end())
        return;

    arr = iter.value();
    arr += socket->readAll();
    }
    while (arr.size() >= 2)
    {
        bool fin = static_cast<quint8>(arr.constData()[0]) & 0x80;
        WebSocketOpcode opcode = static_cast<WebSocketOpcode>(static_cast<quint8>(arr.constData()[0]) & 0x0f);
        bool mask = static_cast<quint8>(arr.constData()[1]) & 0x80;
        quint64 payloadLen = static_cast<quint8>(arr.constData()[1]) & 0x7f;

        int headerLen = 2;
        if (payloadLen == 127) {
            if (arr.size() < 10)
                return;
            payloadLen = qFromBigEndian<quint64>(reinterpret_cast<const uchar *>(arr.constData() + 2));
            headerLen += 8;
        } else if (payloadLen == 126) {
            if (arr.size() < 4)
                return;
            payloadLen = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(arr.constData() + 2));
            headerLen += 2;
        }

        payloadLen += mask ? 4 : 0;
        if (static_cast<quint64>(arr.size()) < payloadLen)
            return;

        arr.remove(0, headerLen);
        QByteArray payload = arr.left(payloadLen);
        arr.remove(0, payloadLen);

        if (mask) {
            for (int i = 4; i < payload.size(); ++i)
                payload[i] = static_cast<quint8>(payload[i]) ^ static_cast<quint8>(payload[i % 4]);
            payload.remove(0, 4);
        }

        switch (opcode) {
        case WS_TEXT:
            if (!fin) {
                qWarning("Fragmented Frame Not Implemented");
            }
            processMessage(QString::fromLatin1(payload.constData()));
            break;
        default:
            qWarning(qPrintable(QString::fromLatin1("Unhandled opcode '%1'").arg(opcode)));
            break;
        }
    }
}

void QHtmlService::onSocketDisconnected()
{
    QTcpSocket * const socket = qobject_cast<QTcpSocket *>(sender());
    if (mDebug)
         qDebug() << "QHtmlService::onSocketDisconnected";

    {
    //QMutexLocker lock(&mWebSocketHandshakeHeadersMutex);
    mWebSocketHandshakeHeaders.remove(socket);
    }
    {
    QMutexLocker lock(&mWebSocketFrameBuffersMutex);
    mWebSocketFrameBuffers.remove(socket);
    }
    socket->deleteLater();
}

QString QHtmlService::translateHttpStatusCode(int statusCode)
{
    switch (statusCode) {
    case 200:
        return QStringLiteral("OK");
    case 400:
        return QStringLiteral("Bad Request");
    case 404:
        return QStringLiteral("Not Found");
    case 426:
        return QStringLiteral("Upgrade Required");
    case 501:
        return QStringLiteral("Not Implemented");
    default:
        return QStringLiteral("");
    }
}

QString QHtmlService::guessMimeType(const QString &suffix)
{
    if (suffix == QStringLiteral("html"))
        return QStringLiteral("text/html");
    else if (suffix == QStringLiteral("js"))
        return QStringLiteral("text/javascript");
    else
        return QStringLiteral("");
}

QByteArray QHtmlService::webSocketFrame(int opcode, const QByteArray &payload)
{
    uchar header[10];
    header[0] = static_cast<quint8>(0x80 | (opcode & 0x0f));

    QByteArray frame;
    if (payload.size() <= 125) {
        qToBigEndian(static_cast<quint8>(payload.size()), &header[1]);
        frame += QByteArray(reinterpret_cast<char *>(header), 2);
    } else if (payload.size() <= 65535) {
        qToBigEndian(static_cast<quint8>(126), &header[1]);
        qToBigEndian(static_cast<quint16>(payload.size()), &header[2]);
        frame += QByteArray(reinterpret_cast<char *>(header), 4);
    } else {
        qToBigEndian(static_cast<quint8>(127), &header[1]);
        qToBigEndian(static_cast<quint64>(payload.size()), &header[2]);
        frame += QByteArray(reinterpret_cast<char *>(header), 10);
    }
    frame += payload;
    return frame;
}

void QHtmlService::sendStaticResponse(QTcpSocket *socket, int statusCode, QString mimeType, QByteArray data) const
{
    QString responseHeader = QString::fromLatin1("HTTP/1.1 %1 %2\r\n").arg(statusCode).arg(translateHttpStatusCode(statusCode));
    if (data.isEmpty()) {
        mimeType = guessMimeType(QStringLiteral("html"));
        data = QString::fromLatin1("<html><head><title>%1 %2</title></header><body>%2</body></html>")
                                   .arg(statusCode).arg(translateHttpStatusCode(statusCode)).toLatin1();
    }
    responseHeader += QString::fromLatin1("Content-Type: %1\r\nContent-Length: %2\r\n").arg(mimeType).arg(data.size());
    responseHeader += QStringLiteral("\r\n");
    socket->write(responseHeader.toLatin1() + data);
    socket->close();
}

void QHtmlService::sendWebSocketHandshake(QTcpSocket *socket)
{
    static const char *SEC_WEBSOCKET_KEY_MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // QMutexLocker lock(&mWebSocketHandshakeHeadersMutex);

    QString key;
    foreach (const QString &line, mWebSocketHandshakeHeaders[socket]) {
        const QString field = line.section(QLatin1Char(':'), 0, 0);
        const QString value = line.section(QLatin1Char(':'), 1, -1).trimmed();

        if (field == QStringLiteral("Upgrade")) {
            if (value.compare(QStringLiteral("websocket"), Qt::CaseInsensitive) != 0) {
                sendStaticResponse(socket, 400);
                return;
            }
        } else if (field == QStringLiteral("Connection")) {
            bool foundUpgrade = false;
            foreach (const QString &item, value.split(QLatin1Char(',')))
                if (item.trimmed().compare(QStringLiteral("Upgrade"), Qt::CaseInsensitive) == 0) {
                    foundUpgrade = true;
                    break;
                }
            if (!foundUpgrade) {
                sendStaticResponse(socket, 400);
                return;
            }
        } else if (field == QStringLiteral("Sec-WebSocket-Key")) {
            key = value;
        } else if (field == QStringLiteral("Sec-WebSocket-Version")) {
            if (value.toInt() < 7) {
                sendStaticResponse(socket, 426);
                return;
            }
        } else if (field == QStringLiteral("Sec-WebSocket-Protocol")) {
            if (value != QStringLiteral("qthtml")) {
                sendStaticResponse(socket, 400);
                return;
            }
        }
    }

    if (key.isEmpty()) {
        sendStaticResponse(socket, 400);
        return;
    }

    key += QString::fromLatin1(SEC_WEBSOCKET_KEY_MAGIC);
    const QByteArray hash = QCryptographicHash::hash(key.toLatin1(), QCryptographicHash::Sha1);
    const QString accept = QString::fromLatin1(hash.toBase64().constData());
    const QString responseHeader = QString::fromLatin1("HTTP/1.1 101 Switching Protocols\r\n"
                                                 "Upgrade: websocket\r\n"
                                                 "Connection: Upgrade\r\n"
                                                 "Sec-WebSocket-Accept: %1\r\n"
                                                 "Sec-WebSocket-Protocol: qthtml\r\n"
                                                 "\r\n").arg(accept);
    socket->write(responseHeader.toLatin1());
    connect(socket, SIGNAL(readyRead()), SLOT(onWebSocketReadyRead()));

    if (mDebug)
         qDebug() << "QHtmlService::sendWebSocketHandshake";

    {
    QMutexLocker lock(&mWebSocketFrameBuffersMutex);
    mWebSocketFrameBuffers.insert(socket, QByteArray());
    }

    {
    QMutexLocker lock(&mWinIdsMutex);
    // initialize commands
    foreach (int winId, mWinIds)
        sendMessage(socket, QString(QChar::fromLatin1(static_cast<char>(MessageCommand::CreateWindow))), winId);
    }
    emit flush();
}

//template <typename... Args>
//void QHtmlService::sendMessage(QTcpSocket *socket, MessageCommand command, Args... args) const
//{
//    sendMessage(socket, QString(QChar::fromLatin1(static_cast<char>(command))), args...);
//}

//template <typename... Args>
//void QHtmlService::sendMessage(MessageCommand command, Args... args) const
//{
//    sendMessage(QString(QChar::fromLatin1(static_cast<char>(command))), args...);
//}

//template <typename T, typename... Args>
//void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message, T arg0, Args... args) const
//{
//    sendMessage(socket, message + QString::fromLatin1(" %1").arg(arg0), args...);
//}

//template <typename T, typename... Args>
//void QHtmlService::sendMessage(const QString &message, T arg0, Args... args) const
//{
//    sendMessage(message + QString::fromLatin1(" %1").arg(arg0), args...);
//}



template <typename T1>
void QHtmlService::sendMessage(const QString &message, T1 t1) const
{
    sendMessage(message + QString::fromLatin1(" %1").arg(t1));
}
template <typename T1, typename T2>
void QHtmlService::sendMessage(const QString &message, T1 t1, T2 t2) const
{
    sendMessage(message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2)
                );
}
template <typename T1, typename T2, typename T3>
void QHtmlService::sendMessage(const QString &message, T1 t1, T2 t2, T3 t3) const
{
    sendMessage(message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3)
                );
}
template <typename T1, typename T2, typename T3, typename T4>
void QHtmlService::sendMessage(const QString &message, T1 t1, T2 t2, T3 t3, T4 t4) const
{
    sendMessage(message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3) +
                QString::fromLatin1(" %1").arg(t4)
                );
}
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void QHtmlService::sendMessage(const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const
{
    sendMessage(message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3) +
                QString::fromLatin1(" %1").arg(t4) +
                QString::fromLatin1(" %1").arg(t5)
                );
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void QHtmlService::sendMessage(const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) const
{
    sendMessage(message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3) +
                QString::fromLatin1(" %1").arg(t4) +
                QString::fromLatin1(" %1").arg(t5) +
                QString::fromLatin1(" %1").arg(t6)
                );
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void QHtmlService::sendMessage(const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) const
{
    sendMessage(message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3) +
                QString::fromLatin1(" %1").arg(t4) +
                QString::fromLatin1(" %1").arg(t5) +
                QString::fromLatin1(" %1").arg(t6) +
                QString::fromLatin1(" %1").arg(t7)
                );
}

template <typename T1>
void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message, T1 t1) const
{
    sendMessage(socket, message + QString::fromLatin1(" %1").arg(t1));
}
template <typename T1, typename T2>
void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message, T1 t1, T2 t2) const
{
    sendMessage(socket, message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2)
                );
}
template <typename T1, typename T2, typename T3>
void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message, T1 t1, T2 t2, T3 t3) const
{
    sendMessage(socket, message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3)
                );
}
template <typename T1, typename T2, typename T3, typename T4>
void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message, T1 t1, T2 t2, T3 t3, T4 t4) const
{
    sendMessage(socket, message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3) +
                QString::fromLatin1(" %1").arg(t4)
                );
}
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const
{
    sendMessage(socket, message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3) +
                QString::fromLatin1(" %1").arg(t4) +
                QString::fromLatin1(" %1").arg(t5)
                );
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) const
{
    sendMessage(socket, message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3) +
                QString::fromLatin1(" %1").arg(t4) +
                QString::fromLatin1(" %1").arg(t5) +
                QString::fromLatin1(" %1").arg(t6)
                );
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) const
{
    sendMessage(socket, message +
                QString::fromLatin1(" %1").arg(t1) +
                QString::fromLatin1(" %1").arg(t2) +
                QString::fromLatin1(" %1").arg(t3) +
                QString::fromLatin1(" %1").arg(t4) +
                QString::fromLatin1(" %1").arg(t5) +
                QString::fromLatin1(" %1").arg(t6) +
                QString::fromLatin1(" %1").arg(t7)
                );
}




void QHtmlService::sendMessage(QTcpSocket *socket, const QString &message) const
{
    const QByteArray payload = message.toLatin1();
    socket->write(webSocketFrame(WS_TEXT, payload));
}

void QHtmlService::sendMessage(const QString &message) const
{
    const QByteArray payload = message.toLatin1();
    QMutexLocker lock(&mWebSocketFrameBuffersMutex);
    for ( QHash<QTcpSocket *, QByteArray>::ConstIterator iter = mWebSocketFrameBuffers.constBegin(); iter != mWebSocketFrameBuffers.constEnd(); ++iter)
        iter.key()->write(webSocketFrame(WS_TEXT, payload));
}

void QHtmlService::processMessage(const QString &message) const
{
    const QStringList args = message.split(QLatin1Char(' '));
    MessageCommand command = static_cast<MessageCommand>(args[0][0].toLatin1());

    switch (command) {
    case MessageCommand::SetScreenGeometry:
        mScreenGeometry = QRect(args[1].toInt(), args[2].toInt(),
                                args[3].toInt(), args[4].toInt());
        emit setScreenGeometry(mScreenGeometry.x(), mScreenGeometry.y(),
                               mScreenGeometry.width(), mScreenGeometry.height());
        break;
    case MessageCommand::DestroyWindow:
        emit destroy(args[1].toInt());
        break;
    case MessageCommand::WindowActivated:
        emit activated(args[1].toInt());
        break;
    case MessageCommand::SetGeometry:
        emit setWindowGeometry(args[1].toInt(),
                               args[2].toInt(), args[3].toInt(),
                               args[4].toInt(), args[5].toInt());
        break;
    case MessageCommand::KeyDown:
        emit keyEvent(args[1].toInt(), QEvent::KeyPress,
                      args[2].toInt(), args[3].toInt(),
                      args[4].toInt() ? QString(args[2].toInt()) : QStringLiteral(""));
        break;
    case MessageCommand::KeyUp:
        emit keyEvent(args[1].toInt(), QEvent::KeyRelease,
                      args[2].toInt(), args[3].toInt(),
                      args[4].toInt() ? QString(args[2].toInt()) : QStringLiteral(""));
        break;
    case MessageCommand::MouseMove:
    case MessageCommand::MouseDown:
    case MessageCommand::MouseUp:
        emit mouseEvent(args[1].toInt(),
                        args[2].toInt(), args[3].toInt(),
                        args[4].toInt(), args[5].toInt(),
                        args[6].toInt(), args[7].toInt());
        break;
    case MessageCommand::MouseWheel:
        emit mouseWheel(args[1].toInt(),
                        args[2].toInt(), args[3].toInt(),
                        args[4].toInt(), args[5].toInt(),
                        args[6].toInt(), args[7].toInt());
        break;
    default:
        qWarning(qPrintable(QString::fromLatin1("Unhandled Message '%1'").arg(message)));
        break;
    }
}

QT_END_NAMESPACE
