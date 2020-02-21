# -----------------------------------------------------------
# Project file
# Copyright (c) 2009-2018 Avinger, Inc
# -----------------------------------------------------------

TARGET = octConsole

TEMPLATE = app

QT += charts
QT += sql xml

# XXX this needs to be here to link even though we do not
# have any openGL code.  QtSingleApplication::sendMessage
# craps out with an unresolved external symbol (SendMessageTimeoutW)
# It's all a little odd.
#win32 {
#   QT += opengl
#}

CONFIG += c++14

DEFINES += QT_XML_LIB QT_SQL_LIB

#isEmpty(OPENCL_DIR):OPENCL_DIR=$$(OPENCL_DIR)

#isEmpty(OPENCL_DIR) {
#    message("set OPENCL_DIR as environment variable or qmake variable to get rid of this message")
    OPENCL_DIR = "C:/Program Files (x86)/IntelSWTools/sw_dev_tools/OpenCL/sdk"
#}

#    CUDA_DIR1 = "C:\Users\lvincze\Documents\GitHub\cudaUnitTest\cudaFFT"
#    CUDA_DIR2 = "C:\CUDA\lib\x64"
#    CUDA_DIR3 = "C:\Users\lvincze\Documents\GitHub\bin\win64"

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
       $$OPENCL_DIR/include \
       ../../lib/win32/DataTranslation/Include \
       Backend                              \
       Include                              \
       Frontend/GeneratedFiles              \
       Frontend/Widgets                     \
       Frontend

     INCLUDEPATH *= $$CUDA_DIR1

    LIBS += -L$$OPENCL_DIR/lib/x64 -lopencl
#    LIBS += -L$$CUDA_DIR3/Debug -lcudaFFT
#    LIBS += -L$$CUDA_DIR2 -lcudart -lcufft


    LIBS +=                                       \
       -L"../../lib/win32/DataTranslation/"               \
       -lglu32                                            \
       -luser32

}  #win32

unix {

#   INCLUDEPATH +=                           \
#       ../../lib/linux32/ipp/6.1.0.039/ia32/include

   INCLUDEPATH +=                           \
       .                                    \
       ../../Common/Include                 \
        Backend                              \
       Include                              \
       Frontend/GeneratedFiles              \
       Frontend/Widgets                     \
       Frontend

    OBJECTS_DIR = ./.obj
    MOC_DIR = ./.moc
}


DEPENDPATH += .
UI_DIR  += ./Frontend/GeneratedFiles
RCC_DIR += ./Frontend/GeneratedFiles

#Include file(s)
include( $${TARGET}.pri )

#CONFIG( release ) {
#    QMAKE_CLEAN += release\\$${TARGET}.map release\\$${TARGET}.pdb release\\$${TARGET}.key
#}

RESOURCES += \
    OpenClResources.qrc
