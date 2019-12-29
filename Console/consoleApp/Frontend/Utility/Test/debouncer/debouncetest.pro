TEMPLATE = app
TARGET = debouncetest
DESTDIR = .
CONFIG += qtestlib
INCLUDEPATH += ../ ../../../include
DEPENDPATH += .
HEADERS += debounceTest.h
SOURCES += debounceTest.cpp ../../debouncer.cpp
