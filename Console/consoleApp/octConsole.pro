# -----------------------------------------------------------
# Project file
# Copyright (c) 2009-2018 Avinger, Inc
# -----------------------------------------------------------

TARGET = octConsole

TEMPLATE = app

QT += sql xml

# XXX this needs to be here to link even though we do not
# have any openGL code.  QtSingleApplication::sendMessage
# craps out with an unresolved external symbol (SendMessageTimeoutW)
# It's all a little odd.
win32 {
   QT += opengl
}

CONFIG += c++11
CONFIG += qxt
QXT += core gui

DEFINES += QT_XML_LIB QT_SQL_LIB  QWT_DLL

win32 {
   # Turn on additional settings for building the Windows release
   #   Zi   Enable debugging info
   QMAKE_CFLAGS_RELEASE   += /Zi
   QMAKE_CXXFLAGS_RELEASE += /Zi
   
   # Turn on additional settings for linking the Windows release
   #   DEBUG    Create debug information
   #   MAP  Create memory map
   #   OPT:REF  Eliminate functions and/or data that are not referenced
   #   OPT:ICF  Perform identical COMDAT folding (identical code is only referenced once)
   QMAKE_LFLAGS_RELEASE += /DEBUG /MAP /OPT:REF /OPT:ICF 

   INCLUDEPATH +=                           \
       .                                    \
       ..                                   \
       ../../lib/win32/ffmpeg/include                   \
       ../../lib/win32/ffmpeg/include/libavcodec        \
       ../../lib/win32/ffmpeg/include/libavformat       \
       ../../lib/win32/ffmpeg/include/libswscale        \
       ../../Common/Include                 \
       ../../lib/win32/AlazarTech/Include/C          \
       ../../lib/win32/Qwt-6.0.1/src                 \
       ../../lib/win32/Qxt-0.6.0/include/QxtCore     \
       ../../lib/win32/Qxt-0.6.0/include/QxtGui      \
       ../../lib/win32/ipp/ia32/include              \
       ../../lib/win32/qserialport/include/QtSerialPort \
       ../../lib/win32/AMD/AMD-APP/include  \
       ../../lib/win32/AMD/clAmdFft/include \
       ../../lib/win32/DataTranslation/Include \
       Backend                              \
       Include                              \
       Frontend/GeneratedFiles              \
       Frontend/Widgets                     \
       Frontend

    LIBS += -L"../../lib/win32/AMD/AMD-APP/lib/x86/" -lopencl
    LIBS += -L"../../lib/win32/AMD/clAmdFft/lib"
    LIBS += -L"../../lib/win32/AMD/clAmdFft/bin"
    LIBS += -L../../lib/win32/ffmpeg/lib
    LIBS += -L../../lib/win32/ffmpeg/bin
    LIBS +=                                       \
       -L"../../lib/win32/AlazarTech/lib/x86"             \
       -L"../../lib/win32/Qwt-6.0.1/lib"                  \
       -L"../../lib/win32/Qxt-0.6.0/lib"                  \
       -L"../../lib/win32/ipp/ia32/stublib"               \
       -L"../../lib/win32/qserialport/lib"                \
       -L"../../lib/win32/DataTranslation/"               \
       -l../../lib/win32/AlazarTech/lib/x86/ATSApi        \
       -l../../lib/win32/ipp/ia32/stublib/ippcore         \
       -l../../lib/win32/ipp/ia32/stublib/ippi            \
       -l../../lib/win32/ipp/ia32/stublib/ipps            \
       -l../../lib/win32/ipp/ia32/stublib/ippsr           \
       -lglu32                                            \
       -lQtSerialPort1                                    \
       -loldaapi32                                        \
       -lolmem32                                          \
       -lgraph32                                          \
       -luser32                                           \
       -lClAmdFFt.Runtime
   LIBS += -lswscale -lavformat -lavcodec -lavutil

   CONFIG( debug, debug|release ) {
       LIBS += -lqwtd -lqxtguid -lqxtcored
   } else {
       LIBS += -lqwt -lqxtgui -lqxtcore
   }
}  #win32

unix {
   INCLUDEPATH +=                           \
       .                                    \
       ../../Common/Include                 \
       ../../lib/linux32/AlazarTech/include    \
       ../../lib/linux32/qwt/include \
       ../../lib/linux32/ipp/6.1.0.039/ia32/include \
       ../../lib/linux32/libqxt/include/QxtCore \
       ../../lib/linux32/libqxt/include/QxtGui \
       ../../lib/linux32/qserialport/include/QtSerialPort \
       Backend                              \
       Include                              \
       Frontend/GeneratedFiles              \
       Frontend/Widgets                     \
       Frontend

   LIBS += \
       -L"../../lib/linux32/AlazarTech/lib" \
       -L"../../lib/linux32/qwt/lib" \
       -L"../../lib/linux32/libqxt/lib" \
       -L"../../lib/linux32/ipp/6.1.0.039/ia32/sharedlib" \
       -L"../../lib/linux32/qserialport/lib" \
       -lqwt \
       -lQxtGui \
       -lQxtCore \
       -lippdc \
       -lippcc \
       -lippac \
       -lippsr \
       -lippvc \
       -lippcv \
       -lippj \
       -lippi \
       -lipps \
       -lippsc \
       -lippcore \
       -liomp5 \
       -lPlxApi \
       -lQtSerialPort

    OBJECTS_DIR = ./.obj
    MOC_DIR = ./.moc
}


DEPENDPATH += .
UI_DIR  += ./Frontend/GeneratedFiles
RCC_DIR += ./Frontend/GeneratedFiles

#Include file(s)
include( $${TARGET}.pri )

CONFIG( release ) {
    QMAKE_CLEAN += release\\$${TARGET}.map release\\$${TARGET}.pdb release\\$${TARGET}.key
}

unix|win32: LIBS += -L$$PWD/../../lib/win32/FTDI/ -lftd2xx

INCLUDEPATH += $$PWD/../../lib/win32/FTDI
DEPENDPATH += $$PWD/../../lib/win32/FTDI

HEADERS += \
    ../../lib/win32/AlazarTech/Include/C/AlazarApiType.h \
    ../../lib/win32/AlazarTech/Include/C/AlazarApiFunction.h
