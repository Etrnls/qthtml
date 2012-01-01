TARGET = htmlservice

QT += network serviceframework

QMAKE_CXXFLAGS += -std=c++0x

DEFINES += \
           QT_NO_CAST_FROM_BYTEARRAY \
           QT_NO_CAST_FROM_ASCII \
           QT_NO_CAST_TO_ASCII

HEADERS += \
           qhtmlservice.h
SOURCES += \
           main.cpp \
           qhtmlservice.cpp

RESOURCES += html.qrc
