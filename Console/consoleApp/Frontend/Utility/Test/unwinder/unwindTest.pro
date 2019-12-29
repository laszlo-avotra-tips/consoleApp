TEMPLATE = app
TARGET = unwindTest
DESTDIR = .
CONFIG += qtestlib
INCLUDEPATH += ../ ../../../include
DEPENDPATH += .
HEADERS += unwindTest.h
SOURCES += unwindTest.cpp ../unwindMachine.cpp ../debouncer.cpp
