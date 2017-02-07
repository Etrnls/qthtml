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

#ifndef QHTMLSERVICE_H
#define QHTMLSERVICE_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QVector>
#include <QtCore/QRect>
#include <QtCore/QMutex>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QTcpServer;
class QTcpSocket;

class QHtmlService : public QObject
{
    Q_OBJECT
public:
    QHtmlService();
    ~QHtmlService();

    // PlatformScreen Services
    Q_INVOKABLE int getScreenWidth() const;
    Q_INVOKABLE int getScreenHeight() const;

    // PlatformCursor Services
    Q_INVOKABLE void changeCursor(int shape) const;

    // PlatformWindow Services
    Q_INVOKABLE int generateWinId(int instance_id);
    Q_INVOKABLE bool drawJSWindow(int winId, int windowType);
    Q_INVOKABLE void deallocateWinId(int winId, int instance_id);
    Q_INVOKABLE void setGeometry(int winId, int x, int y, int width, int height, int instance_id) const;
    Q_INVOKABLE void setVisible(int winId, bool visible, int instance_id) const;
    Q_INVOKABLE void setWindowTitle(int winId, const QString &title, int instance_id) const;
    Q_INVOKABLE void raise(int winId, int instance_id) const;

    // PlatformBackingStore Services
    Q_INVOKABLE void flush(int winId, int x, int y, int width, int height, const QByteArray &imageData) const;
    Q_INVOKABLE void scroll(int winId, int x, int y, int width, int height, int dx, int dy) const;
signals:
    // PlatformScreen Services
    void setScreenGeometry(int x, int y, int width, int height) const;

    // PlatformWindow Services
    void destroy(int winId, int instance_id) const;
    void activated(int winId, int instance_id) const;
    void setWindowGeometry(int winId, int x, int y, int width, int height, int instance_id) const;
    void keyEvent(int winId, int type, int code, int modifiers, const QString &text, int instance_id) const;
    void mouseEvent(int winId, int localX, int localY, int globalX, int globalY, int buttons, int modifiers, int instance_id) const;
    void mouseWheel(int winId, int localX, int localY, int globalX, int globalY, int delta, int modifiers, int instance_id) const;

    // PlatformBackingStore Services
    void flush() const;
private slots:
    void onServerNewConnection();
    void onHttpSocketReadyRead();
    void onSocketDisconnected();
    void onSocketActivated(int socket_fd);

private:
    void webSocketReadyRead(QTcpSocket *socket);
    enum WebSocketOpcode {
        WS_CONTINUATION = 0,
        WS_TEXT = 1,
        WS_BINARY = 2,
        WS_CLOSE = 8,
        WS_PING = 9,
        WS_PONG = 10
    };

    enum MessageCommand {
        SetScreenGeometry = 's',    // width, height
        ChangeCursor = 'c',         // shape
        CreateWindow = 'w',         // winId
        DestroyWindow = 'W',        // winId
        WindowActivated = 'a',      // winId
        SetGeometry = 'g',          // winId, x, y, width, height
        SetVisible = 'v',           // winId, visible
        SetWindowTitle = 't',       // winId, title
        Raise = 'r',                // winId
        Flush = 'f',                // winId, x, y, width, height, url
        Scroll = '+',               // winId, x, y, width, height, dx, dy
        KeyDown = 'k',              // winId, keyCode, modifiers
        KeyUp = 'K',                // winId, keyCode, modifiers
        MouseMove = '~',            // winId, localX, localY, globalX, globalY, buttons, modifiers
        MouseDown = 'm',            // winId, localX, localY, globalX, globalY, buttons, modifiers
        MouseUp = 'M',              // winId, localX, localY, globalX, globalY, buttons, modifiers
        MouseWheel = 'w',           // winId, localX, localY, globalX, globalY, delta, modifiers
        WSDebug = 'D'               // just for websocket debugging
    };

    // Helper Functions
    static QString translateHttpStatusCode(int statusCode);
    static QString guessMimeType(const QString &suffix);

    static QByteArray webSocketFrame(int opcode, const QByteArray &payload);

    // HTTP Protocols
    void sendStaticResponse(QTcpSocket *socket, int statusCode, QString mimeType = QString(), QByteArray data = QByteArray()) const;
    void sendWebSocketHandshake(QTcpSocket *socket);

    // Qt HTTP Platform WebSocket Protocols
    // yes. its stupid, but it works on VS2010 ))
    void sendMessage(const QString &message) const;
    template <typename T1>
    void sendMessage(const QString &message, T1 t1) const;
    template <typename T1, typename T2>
    void sendMessage(const QString &message, T1 t1, T2 t2) const;
    template <typename T1, typename T2, typename T3>
    void sendMessage(const QString &message, T1 t1, T2 t2, T3 t3) const;
    template <typename T1, typename T2, typename T3, typename T4>
    void sendMessage(const QString &message, T1 t1, T2 t2, T3 t3, T4 t4) const;
    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    void sendMessage(const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const;
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void sendMessage(const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) const;
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void sendMessage(const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) const;

    void sendMessage(QTcpSocket *socket, const QString &message) const;
    template <typename T1>
    void sendMessage(QTcpSocket *socket,const QString &message, T1 t1) const;
    template <typename T1, typename T2>
    void sendMessage(QTcpSocket *socket,const QString &message, T1 t1, T2 t2) const;
    template <typename T1, typename T2, typename T3>
    void sendMessage(QTcpSocket *socket,const QString &message, T1 t1, T2 t2, T3 t3) const;
    template <typename T1, typename T2, typename T3, typename T4>
    void sendMessage(QTcpSocket *socket,const QString &message, T1 t1, T2 t2, T3 t3, T4 t4) const;
    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    void sendMessage(QTcpSocket *socket,const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const;
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void sendMessage(QTcpSocket *socket,const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) const;
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void sendMessage(QTcpSocket *socket,const QString &message, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) const;

    void processMessage(const QString &message) const;
    bool isAllowedWinId(int win_id, int instance_id) const;
    int fromWinIdToInstanceId(int winId) const;

    QTcpServer *mpServer;
    QHash<QTcpSocket *, QStringList> mWebSocketHandshakeHeaders;
    QHash<QTcpSocket *, QByteArray> mWebSocketFrameBuffers;

    mutable QRect mScreenGeometry;

    typedef enum
    {
        not_showed = 0,
        showed,
        awaiting_removal
    } tWinState;

    //QHtmlWindow instance id ,win id, win state
    typedef struct
    {
        int instance_id;    //QHtmlWindow instance id
        int windId;         //win id
        tWinState st;       //win state
    } tWinIdStruct;
    QVector<tWinIdStruct>  mWinIds;
    mutable QMutex      mWinIdsMutex;

    //QMutex mWebSocketHandshakeHeadersMutex;
    mutable QMutex mWebSocketFrameBuffersMutex;

    bool mDebug;
};

QT_END_NAMESPACE

#endif
