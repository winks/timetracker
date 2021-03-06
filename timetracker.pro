#-------------------------------------------------
#
# Project created by QtCreator 2018-03-07T22:34:00
#
#-------------------------------------------------

QT += core gui dbus sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
	CONFIG -= release
	DEBUG_RELEASE = debug
} else {
	CONFIG -= debug
	DEBUG_RELEASE = release
}
CONFIG += c++11

TARGET = timetracker
TEMPLATE = app

DESTDIR     = bin
OBJECTS_DIR = build/$$DEBUG_RELEASE
MOC_DIR     = build/gen/moc
UI_DIR      = build/gen/ui
RCC_DIR     = build/gen/rcc

VERSION = 0.1.0.0
RC_ICONS = resources/icon.ico
QMAKE_TARGET_COMPANY = f5n.org
QMAKE_TARGET_PRODUCT = TimeTracker
QMAKE_TARGET_DESCRIPTION = tracking times
QMAKE_TARGET_COPYRIGHT = something 2018

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_NAME=\\\"$$QMAKE_TARGET_PRODUCT\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/main.cpp \
    src/ui/aboutdialog.cpp \
    src/ui/mainwindow.cpp \

HEADERS += \
    src/ui/aboutdialog.h \
    src/ui/mainwindow.h \

FORMS += \
    src/ui/aboutdialog.ui \
    src/ui/mainwindow.ui \

RESOURCES += \
    src/timetracker.qrc

DBUS_ADAPTORS += \
    src/timetracker.xml
