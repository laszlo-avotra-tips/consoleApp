TEMPLATE = app
TARGET = trigtabletest
DESTDIR = .
CONFIG += qtestlib
INCLUDEPATH += ../ ../../../include
DEPENDPATH += .
HEADERS += trigtabletest.h
SOURCES += trigtabletest.cpp ../trigLookupTable.cpp
