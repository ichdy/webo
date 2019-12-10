#-------------------------------------------------
#
# Project created by QtCreator 2019-02-12T08:03:15
#
#-------------------------------------------------

QT       += core gui widgets network

CONFIG += c++11
TARGET = Webo
TEMPLATE = app
OUT_PWD = $$PWD/bundle

DEFINES += QT_DEPRECATED_WARNINGS

win32 {
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
    DEFINES += _ATL_XP_TARGETING
}

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/itemdelegate.cpp \
    src/parametersdialog.cpp \
    src/aboutdialog.cpp \
    src/consoledialog.cpp \
    src/downloaddialog.cpp \
    src/nodejs.cpp \
    src/simplenam.cpp

HEADERS += \
    src/mainwindow.h \
    src/itemdelegate.h \
    src/parametersdialog.h \
    src/aboutdialog.h \
    src/consoledialog.h \
    src/downloaddialog.h \
    src/nodejs.h \
    src/simplenam.h

RESOURCES += \
    Webo.qrc

RC_FILE = Webo.rc
