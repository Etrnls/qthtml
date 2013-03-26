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

#ifndef QHTMLCURSOR_H
#define QHTMLCURSOR_h

#include <qpa/QPlatformCursor.h>

QT_BEGIN_NAMESPACE

class QHtmlScreen;

class QHtmlCursor : public QPlatformCursor
{
    Q_OBJECT
public:
    QHtmlCursor(QHtmlScreen *screen, QObject *htmlService);
    
    void changeCursor(QCursor *windowCursor, QWindow *window);
private:
    QObject *mHtmlService;
};

QT_END_NAMESPACE

#endif
