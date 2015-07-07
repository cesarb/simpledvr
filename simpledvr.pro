TEMPLATE = app

QT += qml quick widgets

CONFIG += c++14 link_pkgconfig
PKGCONFIG += Qt5GStreamerQuick-1.0 gstreamer-1.0

SOURCES += main.cpp \
    pipeline.cpp

HEADERS += \
    pipeline.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
