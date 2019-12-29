# -------------------------------------------------
# Project created by QtCreator 2010-06-29T10:35:01
# -------------------------------------------------
QT += testlib gui
TARGET = dsptest
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    ../../dsp.cpp \
    ../stubs/daqSettings.cpp \
    ../stubs/deviceSettings.cpp \
    ../stubs/logger.cpp

INCLUDEPATH += . \
    ../.. \
    ../stubs \
    ../../../../../lib/win32/AlazarTech/Include/C \
    ../../../../../lib/win32/ipp/ia32/include \
    ../../../../../Common/Include \
    ../../../Include \
    ../../../Frontend \
    ../../../Frontend/Utility

HEADERS += ../../dsp.h \
    ../../../../../Common/Include/sawFile.h \
    ../../../../../Common/Include/util.h \
    ../../../../../Common/Include/defaults.h \
    ../../../../../Common/Include/deviceSettings.h

LIBS += -L"../../../../lib/win32/AlazarTech/lib/x86" \
    -L"../../../../../lib/win32/ipp/ia32/stublib" \
    -L"../../../../../lib/win32/vld/lib"    \
    -l../../../../../lib/win32/AlazarTech/lib/x86/ATSApi \
    -l../../../../../lib/win32/ipp/ia32/stublib/ippcore \
    -l../../../../../lib/win32/ipp/ia32/stublib/ippi \
    -l../../../../../lib/win32/ipp/ia32/stublib/ipps \
    -l../../../../../lib/win32/ipp/ia32/stublib/ippsr
