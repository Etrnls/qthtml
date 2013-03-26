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
}

QHtmlBackingStore::~QHtmlBackingStore()
{
}

QPaintDevice *QHtmlBackingStore::paintDevice()
{
    return &mImage;
}

void QHtmlBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(window);
    Q_UNUSED(offset);
    foreach (const QRect &rect, (region & mDirtyRegion).rects())
        flush(rect);
    mDirtyRegion -= region;
}

void QHtmlBackingStore::resize(const QSize &size, const QRegion &staticContents)
{
    Q_UNUSED(staticContents);
    if (mImage.size() != size)
        mImage = QImage(size, QImage::Format_ARGB32_Premultiplied);
    mDirtyRegion = QRegion();
}

extern void qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset);

bool QHtmlBackingStore::scroll(const QRegion &area, int dx, int dy)
{
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
    window()->handle()->setGeometry(window()->geometry());
    window()->handle()->setVisible(window()->isVisible());
    flush(QRect(QPoint(0, 0), window()->geometry().size()));
}

void QHtmlBackingStore::flush(const QRect &rect)
{
    const QImage &subImage = mImage.copy(rect);
    QBuffer imageDataBuffer;
    subImage.save(&imageDataBuffer, "png");
    imageDataBuffer.close();

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
    mDirtyRegion += region;
    QPlatformBackingStore::beginPaint(region);
}

void QHtmlBackingStore::endPaint()
{
    QPlatformBackingStore::endPaint();
}

QT_END_NAMESPACE
