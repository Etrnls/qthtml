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

#include "qhtmlwindow.h"

#include <QtCore/QtDebug>
#include <qpa/qwindowsysteminterface.h>

QT_BEGIN_NAMESPACE

QHtmlWindow::QHtmlWindow(QWindow *window, QObject *htmlService)
    : QPlatformWindow(window),
    mHtmlService(htmlService)
{
    QMetaObject::invokeMethod(mHtmlService, "allocateWinId",
                              Q_RETURN_ARG(int, mWinId),
                              Q_ARG(int, static_cast<int>(window->type())));

    connect(mHtmlService, SIGNAL(destroy(int)),
                          SLOT(onDestroy(int)));
    connect(mHtmlService, SIGNAL(activated(int)),
                          SLOT(onActivated(int)));
    connect(mHtmlService, SIGNAL(setWindowGeometry(int, int, int, int, int)),
                          SLOT(onSetGeometry(int, int, int, int, int)));

    connect(mHtmlService, SIGNAL(keyEvent(int, int, int, int, QString)),
                          SLOT(onKeyEvent(int, int, int, int, QString)));
    connect(mHtmlService, SIGNAL(mouseEvent(int, int, int, int, int, int, int)),
                          SLOT(onMouseEvent(int, int, int, int, int, int, int)));
    connect(mHtmlService, SIGNAL(mouseWheel(int, int, int, int, int, int, int)),
                          SLOT(onMouseWheel(int, int, int, int, int, int, int)));

    const QRect rect = window->geometry();
    QMetaObject::invokeMethod(mHtmlService, "setGeometry",
                              Q_ARG(int, mWinId),
                              Q_ARG(int, rect.x()),
                              Q_ARG(int, rect.y()),
                              Q_ARG(int, rect.width()),
                              Q_ARG(int, rect.height()));
}

QHtmlWindow::~QHtmlWindow()
{
    QMetaObject::invokeMethod(mHtmlService, "deallocateWinId",
                              Q_ARG(int, mWinId));
}

void QHtmlWindow::setGeometry(const QRect &rect)
{
    QPlatformWindow::setGeometry(rect);
    QMetaObject::invokeMethod(mHtmlService, "setGeometry",
                              Q_ARG(int, mWinId),
                              Q_ARG(int, rect.x()),
                              Q_ARG(int, rect.y()),
                              Q_ARG(int, rect.width()),
                              Q_ARG(int, rect.height()));
}

QMargins QHtmlWindow::frameMargins() const
{
    // TODO
    return QPlatformWindow::frameMargins();
}

void QHtmlWindow::setVisible(bool visible)
{
    QMetaObject::invokeMethod(mHtmlService, "setVisible",
                              Q_ARG(int, mWinId),
                              Q_ARG(bool, visible));
}

WId QHtmlWindow::winId() const
{
    return static_cast<WId>(mWinId);
}

void QHtmlWindow::setWindowTitle(const QString &title)
{
    QMetaObject::invokeMethod(mHtmlService, "setWindowTitle",
                              Q_ARG(int, mWinId),
                              Q_ARG(QString, title));
}

void QHtmlWindow::raise()
{
    QMetaObject::invokeMethod(mHtmlService, "raise",
                              Q_ARG(int, mWinId));
}

void QHtmlWindow::onDestroy(int winId)
{
    if (winId == mWinId) {
        QWindowSystemInterface::handleCloseEvent(window());
    }
}

void QHtmlWindow::onActivated(int winId)
{
    if (winId == mWinId) {
        QWindowSystemInterface::handleWindowActivated(window());
    }
}

void QHtmlWindow::onSetGeometry(int winId, int x, int y, int width, int height)
{
    if (winId == mWinId) {
        const QRect rect(x, y, width, height);
        QPlatformWindow::setGeometry(rect);
        QWindowSystemInterface::handleGeometryChange(window(), rect);
    }
}

void QHtmlWindow::onKeyEvent(int winId, int type, int code, int modifiers, const QString &text)
{
    if (winId == mWinId) {
        QWindowSystemInterface::handleKeyEvent(window(), QEvent::Type(type), code, Qt::KeyboardModifiers(modifiers), text);
    }
}

void QHtmlWindow::onMouseEvent(int winId, int localX, int localY, int globalX, int globalY, int buttons, int modifiers)
{
    if (winId == mWinId) {
        QWindowSystemInterface::handleMouseEvent(window(), QPoint(localX, localY), QPoint(globalX, globalY),
                                                 Qt::MouseButtons(buttons), Qt::KeyboardModifiers(modifiers));
    }
}

void QHtmlWindow::onMouseWheel(int winId, int localX, int localY, int globalX, int globalY, int delta, int modifiers)
{
    if (winId == mWinId) {
        delta *= 120;
        QWindowSystemInterface::handleWheelEvent(window(), QPoint(localX, localY), QPoint(globalX, globalY),
                                                 delta, Qt::Vertical, Qt::KeyboardModifiers(modifiers));
    }
}

QT_END_NAMESPACE
