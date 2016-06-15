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

#ifndef QHTMLWINDOW_H
#define QHTMLWINDOW_H

//#include <qpa/QPlatformWindow.h>
#include <qpa/qplatformwindow.h>

QT_BEGIN_NAMESPACE

class QHtmlWindow : public QObject, public QPlatformWindow
{
    Q_OBJECT
public:
    QHtmlWindow(QWindow *window, QObject *htmlService);
    ~QHtmlWindow();

    void setGeometry(const QRect &rect);

    QMargins frameMargins() const;

    void setVisible(bool visible);

    WId winId() const;

    void setWindowTitle(const QString &title);
    void raise();
public slots:
    void onFlush();
public slots:
    void onDestroy(int winId);
    void onActivated(int winId);
    void onSetGeometry(int winId, int x, int y, int width, int height);
    void onKeyEvent(int winId, int type, int code, int modifiers, const QString &text);
    void onMouseEvent(int winId, int localX, int localY, int globalX, int globalY, int buttons, int modifiers);
    void onMouseWheel(int winId, int localX, int localY, int globalX, int globalY, int delta, int modifiers);
private:
    QObject *mHtmlService;
    bool mDebug;
    bool mDebugEvents;

    int mWinId;
};

QT_END_NAMESPACE

#endif
