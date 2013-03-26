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

#ifndef QHTMLBACKINGSTORE_H
#define QHTMLBACKINGSTORE_H

#include <qpa/qplatformbackingstore.h>
#include <QtGui/QImage>

QT_BEGIN_NAMESPACE

class QHtmlBackingStore : public QObject, public QPlatformBackingStore
{
    Q_OBJECT
public:
    QHtmlBackingStore(QWindow *window, QObject *htmlService);
    ~QHtmlBackingStore();

    QPaintDevice *paintDevice();

    void flush(QWindow *window, const QRegion &region, const QPoint &offset);

    void resize(const QSize &size, const QRegion &staticContents);

    bool scroll(const QRegion &area, int dx, int dy);

    void beginPaint(const QRegion &region);
    void endPaint();
public slots:
    void onFlush();
private:
    void flush(const QRect &rect);

    QObject *mHtmlService;

    QImage mImage;
    QRegion mDirtyRegion;
};

QT_END_NAMESPACE

#endif
