#-------------------------------------------------
#
# Project created by QtCreator 2011-09-28T09:36:02
#
#-------------------------------------------------

QT       += core gui

TARGET = touchKeyboard
TEMPLATE = app


FORMS    += mainwindow.ui \
    mainwindow.ui

SOURCES += \
    ../keyboardinputcontext.cpp \
    ../keyboard.cpp \
    mainwindow.cpp \
    main.cpp

HEADERS += \
    ../../../Include/keyboardinputcontext.h \
    ../../../Include/keyboard.h \
    mainwindow.h

INCLUDEPATH += \
    . \
    ../../../Include \
    ..
