# -----------------------------------------------------------
# Project file
# Copyright (c) 2009-2018 Avinger, Inc
# -----------------------------------------------------------

TARGET = octConsole

TEMPLATE = app

QT += sql xml widgets
QT += charts
QT += serialport
QT += multimedia multimediawidgets


CONFIG += c++14

DEFINES += QT_XML_LIB QT_SQL_LIB

OPENCL_DIR = $$PWD/../../lib/amd64/Intel/OpenCL_SDK/6.3
OPENCL_DIR_7 = $$PWD/../../lib/amd64/Intel/OpenCL_SDK/7.0

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
       ..

    INCLUDEPATH += \
       ../../Common/Include                 \
       $$OPENCL_DIR_7/include               \
       Backend                              \
       Include                              \
       Frontend/Widgets                     \
       Frontend

#    LIBS += -L$$OPENCL_DIR_7/lib/x64 -lopencl
    LIBS += -L$$PWD/../../lib/amd64/Intel/OpenCL_SDK/7.0/lib/x64/ -lOpenCL
    LIBS += -L$$PWD/../../lib/amd64/Axsun/lib/ -lAxsunOCTCapture
    LIBS += -L$$PWD/../../lib/amd64/Axsun/lib/ -lAxsunOCTControl_LW
    LIBS += -L$$PWD/../../lib/amd64/FTDI/ -lftd2xx
    LIBS += -L$$PWD/../../lib/amd64/AVideoRecorder/lib/ -lAVideoRecorder


    LIBS +=                                       \
       -lglu32                                    \
       -luser32

}  #win32

unix {

   INCLUDEPATH +=                           \
       .                                    \
       ../../Common/Include                 \
       Backend                              \
       Include                              \
       Frontend/Widgets                     \
       Frontend

    OBJECTS_DIR = ./.obj
    MOC_DIR = ./.moc
}


DEPENDPATH += .

#Include file(s)
include( $${TARGET}.pri )

#CONFIG( release ) {
#    QMAKE_CLEAN += release\\$${TARGET}.map release\\$${TARGET}.pdb release\\$${TARGET}.key
#}

INCLUDEPATH += $$PWD/../../lib/amd64/Axsun
DEPENDPATH += $$PWD/../../lib/amd64/Axsun

INCLUDEPATH += $$PWD/../../lib/amd64/Intel/OpenCL_SDK/7.0/include
DEPENDPATH += $$PWD/../../lib/amd64/Intel/OpenCL_SDK/7.0/include

INCLUDEPATH += $$PWD/../../lib/amd64/FTDI
DEPENDPATH += $$PWD/../../lib/amd64/FTDI

INCLUDEPATH += $$PWD/../../lib/amd64/AVideoRecorder
DEPENDPATH += $$PWD/../../lib/amd64/AVideoRecorder


RESOURCES += \
    OpenClResources.qrc

FORMS +=

HEADERS +=

SOURCES +=
