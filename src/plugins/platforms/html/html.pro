TARGET = qhtml

PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = QHtmlIntegrationPlugin
load(qt_plugin)

QT += serviceframework core-private gui-private platformsupport-private

DEFINES += \
           QT_NO_CAST_FROM_BYTEARRAY \
           QT_NO_CAST_FROM_ASCII \
           QT_NO_CAST_TO_ASCII

HEADERS = \
          qhtmlintegration.h \
          qhtmlscreen.h \
          qhtmlcursor.h \
          qhtmlwindow.h \
          qhtmlbackingstore.h

SOURCES = \
          main.cpp \
          qhtmlintegration.cpp \
          qhtmlscreen.cpp \
          qhtmlcursor.cpp \
          qhtmlwindow.cpp \
          qhtmlbackingstore.cpp

DESTDIR = $$QT.gui.plugins/platforms
target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
