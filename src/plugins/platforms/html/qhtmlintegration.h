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

#ifndef QHTMLINTEGRATION_H
#define QHTMLINTEGRATION_H

#include <QtGui/QPlatformIntegration>

QT_BEGIN_NAMESPACE

class QAbstractEventDispatcher;

class QHtmlIntegration : public QPlatformIntegration
{
public:
    QHtmlIntegration();

    QPlatformWindow *createPlatformWindow(QWindow *window) const;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const;

    QAbstractEventDispatcher *guiThreadEventDispatcher() const;

    QPlatformFontDatabase *fontDatabase() const;
private:
    QAbstractEventDispatcher *mEventDispatcher;

    QScopedPointer<QObject> mHtmlService;

    QScopedPointer<QPlatformScreen> mScreen;
    QScopedPointer<QPlatformFontDatabase> mFontDatabase;
};

QT_END_NAMESPACE

#endif
