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

#include <QtGlobal>
#include "qhtmlscreen.h"
#include "qhtmlcursor.h"

#include <QtCore/QtDebug>
#include <qpa/qwindowsysteminterface.h>

QT_BEGIN_NAMESPACE

QHtmlScreen::QHtmlScreen(QObject *htmlService)
    : mHtmlService(htmlService)
{
    mDebug = true;
    if (mDebug)
        qDebug() << "QHtmlScreen";

    mCursor.reset(new QHtmlCursor(this, mHtmlService));
    connect(mHtmlService, SIGNAL(setScreenGeometry(int, int, int, int)), SLOT(setGeometry(int, int, int, int)));

    int width;
    int height;
    QMetaObject::invokeMethod(mHtmlService, "getScreenWidth",
                              Q_RETURN_ARG(int, width));
    QMetaObject::invokeMethod(mHtmlService, "getScreenHeight",
                              Q_RETURN_ARG(int, height));
    mGeometry = QRect(0, 0, width, height);
}

QHtmlScreen::~QHtmlScreen()
{
}

QRect QHtmlScreen::geometry() const
{
    return mGeometry;
}

int QHtmlScreen::depth() const
{
    return 32;
}

QImage::Format QHtmlScreen::format() const
{
    return QImage::Format_ARGB32_Premultiplied;
}

void QHtmlScreen::setGeometry(int x, int y, int width, int height)
{
    if (mDebug)
        qDebug() << "QHtmlScreen::setGeometry";

    mGeometry = QRect(x, y, width, height);
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    QWindowSystemInterface::handleScreenGeometryChange(screen(), mGeometry, mGeometry);
#else
    QWindowSystemInterface::handleScreenGeometryChange(screen(), mGeometry);
#endif

}

QT_END_NAMESPACE
