/****************************************************************************
**
** Copyright (C) 2011 by Etrnls
** etrnls@gmail.com
**
** This file is part of the Qt HTML platform plugin.
**
** Qt HTML platform plugin is free software: you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public License as
** published by the Free Software Foundation, either version 3 of the License,
** or (at your option) any later version.
**
** Qt HTML platform plugin is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with Qt HTML platform plugin. If not, see
** <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "qhtmlbackingstore.h"

#include <QtCore/QBuffer>
#include <qpa/qplatformwindow.h>
#include <QtCore/QtDebug>

QT_BEGIN_NAMESPACE

QHtmlBackingStore::QHtmlBackingStore(QWindow *window, QObject *htmlService)
    : QPlatformBackingStore(window),
    mHtmlService(htmlService)
{
    connect(mHtmlService, SIGNAL(flush()), SLOT(onFlush()));
    mDebug = false;
    mPrintDebugImages = false;

	bool draw_ret = false;
    QMetaObject::invokeMethod(mHtmlService, "drawJSWindow"
                              , Q_RETURN_ARG(bool, draw_ret)
                              , Q_ARG(int, static_cast<int>(window->winId()))
                              , Q_ARG(int, static_cast<int>(window->type())));
}

QHtmlBackingStore::~QHtmlBackingStore()
{
}

QPaintDevice *QHtmlBackingStore::paintDevice()
{
    if (mDebug)
        qDebug() << "QHtmlBackingStore::paintDevice " << static_cast<int>(window()->winId());

    return &mImage;
}

void QHtmlBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
    if (mDebug)
        qDebug() << "QHtmlBackingStore::flush " << static_cast<int>(window->winId());

    if (mPrintDebugImages) {
        static int c = 0;
        QString filename = QString::fromLatin1("output%1.png").arg(c++, 4, 10, QLatin1Char('0'));
        qDebug() << "QHtmlBackingStore::flush() saving contents to" << filename.toLocal8Bit().constData();
        mImage.save(filename);
    }

    Q_UNUSED(window);
    Q_UNUSED(offset);
    foreach (const QRect &rect, (region & mDirtyRegion).rects())
        flush(rect);
    mDirtyRegion -= region;
}

void QHtmlBackingStore::resize(const QSize &size, const QRegion &staticContents)
{
    if (mDebug)
        qDebug() << "QHtmlBackingStore::resize " << static_cast<int>(window()->winId());

    Q_UNUSED(staticContents);
    if (mImage.size() != size)
        mImage = QImage(size, QImage::Format_ARGB32_Premultiplied);
    mDirtyRegion = QRegion();
}

extern void qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset);

bool QHtmlBackingStore::scroll(const QRegion &area, int dx, int dy)
{
    if (mDebug)
        qDebug() << "QHtmlBackingStore::scroll " << static_cast<int>(window()->winId());

    const QPoint offset(dx, dy);
    foreach (const QRect &rect, area.rects()) {
        QMetaObject::invokeMethod(mHtmlService, "scroll",
                                  Q_ARG(int, static_cast<int>(window()->winId())),
                                  Q_ARG(int, rect.x()),
                                  Q_ARG(int, rect.y()),
                                  Q_ARG(int, rect.width()),
                                  Q_ARG(int, rect.height()),
                                  Q_ARG(int, dx),
                                  Q_ARG(int, dy));
        qt_scrollRectInImage(mImage, rect, offset);
    }
    return true;
}

void QHtmlBackingStore::onFlush()
{
    if (mDebug)
        qDebug() << "QHtmlBackingStore::onFlush " << static_cast<int>(window()->winId());

    window()->handle()->setGeometry(window()->geometry());
    window()->handle()->setVisible(window()->isVisible());
    flush(QRect(QPoint(0, 0), window()->geometry().size()));
}

void QHtmlBackingStore::flush(const QRect &rect)
{
    if (mDebug)
        qDebug() << "QHtmlBackingStore::flush(rect) " << static_cast<int>(window()->winId());

    const QImage &subImage = mImage.copy(rect);
    QBuffer imageDataBuffer;
    subImage.save(&imageDataBuffer, "png");
    imageDataBuffer.close();

    if (mPrintDebugImages) {
        static int c = 0;
        QString filename = QString::fromLatin1("outputrect%1.png").arg(c++, 4, 10, QLatin1Char('0'));
        qDebug() << "QHtmlBackingStore::flush() saving contents to" << filename.toLocal8Bit().constData();
        mImage.save(filename);
    }

    QMetaObject::invokeMethod(mHtmlService, "flush",
                              Q_ARG(int, static_cast<int>(window()->winId())),
                              Q_ARG(int, rect.x()),
                              Q_ARG(int, rect.y()),
                              Q_ARG(int, rect.width()),
                              Q_ARG(int, rect.height()),
                              Q_ARG(QByteArray, imageDataBuffer.data()));
}

void QHtmlBackingStore::beginPaint(const QRegion &region)
{
    if (mDebug)
        qDebug() << "QHtmlBackingStore::beginPaint " << static_cast<int>(window()->winId());

    mDirtyRegion += region;
    QPlatformBackingStore::beginPaint(region);
}

void QHtmlBackingStore::endPaint()
{
    if (mDebug)
        qDebug() << "QHtmlBackingStore::endPaint " << static_cast<int>(window()->winId());

    QPlatformBackingStore::endPaint();
}

QT_END_NAMESPACE
