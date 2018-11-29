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

#include "qhtmlintegration.h"
#include "qhtmlscreen.h"
#include "qhtmlwindow.h"
#include "qhtmlbackingstore.h"

#include <QtCore/QtDebug>
#include <QtCore/QThread>
#include <QtCore/QCoreApplication>
#include <QtGui/private/qguiapplication_p.h>
#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#ifndef Q_OS_WIN
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#else
#include <QtCore/private/qeventdispatcher_win_p.h>
#endif
#else
#include <QtFontDatabaseSupport/private/qgenericunixfontdatabase_p.h>
#ifndef Q_OS_WIN
#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#else
#include <QtCore/private/qeventdispatcher_win_p.h>
#endif
#endif

#include "qhtmlservice.h"

QT_BEGIN_NAMESPACE

QHtmlIntegration::QHtmlIntegration() :
#ifdef Q_OS_WIN
    mEventDispatcher(new QEventDispatcherWin32())
#else
    mEventDispatcher(createUnixEventDispatcher())
#endif
{
    qDebug() <<__FUNCTION__;
    mDebug = true;
    QGuiApplication::instance()->setEventDispatcher(mEventDispatcher);

    mHtmlService.reset( new QHtmlService() );
    if (mHtmlService.data() == NULL)
        exit(0);

    mScreen.reset(new QHtmlScreen(mHtmlService.data()));
    screenAdded(mScreen.data());

    mFontDatabase.reset(new QGenericUnixFontDatabase());
}

QHtmlIntegration::~QHtmlIntegration()
{
    qDebug() << "QHtmlIntegration::~QHtmlIntegration";
}

QPlatformWindow *QHtmlIntegration::createPlatformWindow(QWindow *window) const
{
    if (mDebug)
        qDebug() << "QHtmlIntegration::createPlatformWindow";

    return new QHtmlWindow(window, mHtmlService.data());
}

QPlatformBackingStore *QHtmlIntegration::createPlatformBackingStore(QWindow *window) const
{
    if (mDebug)
        qDebug() << "QHtmlIntegration::createPlatformBackingStore";
    return new QHtmlBackingStore(window, mHtmlService.data());
}

QAbstractEventDispatcher *QHtmlIntegration::createEventDispatcher() const
{
    qDebug() <<__FUNCTION__;
    return mEventDispatcher;
}

QPlatformFontDatabase *QHtmlIntegration::fontDatabase() const
{
    return mFontDatabase.data();
}

QT_END_NAMESPACE
