SOURCES = test.cpp ../../averager.cpp
CONFIG  += qtestlib
INCLUDEPATH += ../.. ../../../include
sources.files = $$SOURCES *.pro
sources.path = .
INSTALLS += sources #target
