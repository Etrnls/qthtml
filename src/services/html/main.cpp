/****************************************************************************
**
** Copyright (C) 2011 by Etrnls
** etrnls@gmail.com
**
** This file is part of the Qt HTML service daemon.
**
** Qt HTML service daemon is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation, either version 3 of the License,
** or (at your option) any later version.
**
** Qt HTML service daemon is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Qt HTML service daemon. If not, see
** <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtServiceFramework/QRemoteServiceRegister>
#include <QtServiceFramework/QServiceManager>

#include "qhtmlservice.h"

static const char *serviceName = "HtmlService";
static const char *interfaceName = "com.nokia.qt.qpa.HtmlService";
static const char *version = "1.0";
static const char *ident = "html_service";

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QRemoteServiceRegister *serviceRegister = new QRemoteServiceRegister();
    QRemoteServiceRegister::Entry entry;
    entry = serviceRegister->createEntry<QHtmlService>(QString::fromLatin1(serviceName),
                                                       QString::fromLatin1(interfaceName),
                                                       QString::fromLatin1(version));

    entry.setInstantiationType(QRemoteServiceRegister::GlobalInstance);
    serviceRegister->publishEntries(QString::fromLatin1(ident));

    // this will start the server and keep it running
    QObject *htmlService = QServiceManager().loadInterface(QString::fromLatin1(interfaceName));
    if (htmlService == NULL)
        exit(0);

    const int returnCode = app.exec();
    delete htmlService;
    delete serviceRegister;
    return returnCode;
}

