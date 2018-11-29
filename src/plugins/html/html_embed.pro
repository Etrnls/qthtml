TARGET = qhtml
#DEFINES += QT_FONTS_ARE_RESOURCES

PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = QHtmlIntegrationPlugin
load(qt_plugin)

QT += core-private gui-private network

lessThan(QT_MAJOR_VERSION, 5) {
    QT += platformsupport-private
} else {
    equals(QT_MAJOR_VERSION, 5): lessThan(QT_MINOR_VERSION, 8): {
        QT += platformsupport-private
    } else {
        QT += fontdatabase_support_private eventdispatcher_support_private
    }
}

CONFIG += qpa/genericunixfontdatabase   #fonts support

#DEFINES += \
#           QT_NO_CAST_FROM_BYTEARRAY \
#           QT_NO_CAST_FROM_ASCII \
#           QT_NO_CAST_TO_ASCII

HEADERS = \
          qhtmlintegration.h \
          qhtmlscreen.h \
          qhtmlcursor.h \
          qhtmlwindow.h \
          qhtmlbackingstore.h \
          qhtmlservice.h


SOURCES = \
          main.cpp \
          qhtmlintegration.cpp \
          qhtmlscreen.cpp \
          qhtmlcursor.cpp \
          qhtmlwindow.cpp \
          qhtmlbackingstore.cpp \
          qhtmlservice.cpp


#DESTDIR = $$QT.gui.plugins/platforms
#target.path += $$[QT_INSTALL_PLUGINS]/platforms
#INSTALLS += target

OTHER_FILES += \
    client.js \
    client.html \
    jquery.min.js \
    jquery-ui.min.js \
    jquery-ui.css

RESOURCES += \
    html.qrc
