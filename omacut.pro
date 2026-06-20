QT += core gui qml quick quickcontrols2 multimedia

CONFIG += c++17 release
TARGET = omacut
TEMPLATE = app

HEADERS += \
    ffmpeg.h \
    thumbworker.h \
    keyframeworker.h \
    thumbprovider.h \
    backend.h

SOURCES += \
    main.cpp \
    ffmpeg.cpp \
    thumbworker.cpp \
    keyframeworker.cpp \
    thumbprovider.cpp \
    backend.cpp

RESOURCES += resources.qrc
