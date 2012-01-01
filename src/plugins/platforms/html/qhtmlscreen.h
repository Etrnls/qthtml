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

#ifndef QHTMLSCREEN_H
#define QHTMLSCREEN_H

#include <QtGui/QPlatformScreen>

QT_BEGIN_NAMESPACE

class QHtmlCursor;

class QHtmlScreen : public QObject, public QPlatformScreen
{
    Q_OBJECT
public:
    QHtmlScreen(QObject *htmlService);
    ~QHtmlScreen();

    QRect geometry() const;
    int depth() const;
    QImage::Format format() const;
public slots:
    void setGeometry(int x, int y, int width, int height);
private:
    QObject *mHtmlService;

    QRect mGeometry;
    QScopedPointer<QHtmlCursor> mCursor;
};

QT_END_NAMESPACE

#endif
