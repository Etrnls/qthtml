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

#include "qhtmlcursor.h"
#include "qhtmlscreen.h"

#include <QtCore/QtDebug>

QHtmlCursor::QHtmlCursor(QHtmlScreen *screen, QObject *htmlService)
    : QPlatformCursor(),
      mHtmlService(htmlService)
{
}

void QHtmlCursor::changeCursor(QCursor *windowCursor, QWindow *window)
{
    Q_UNUSED(window);
    QMetaObject::invokeMethod(mHtmlService, "changeCursor",
                              Q_ARG(int, static_cast<int>(windowCursor->shape())));
}

