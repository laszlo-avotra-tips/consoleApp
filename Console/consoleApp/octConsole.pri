# -----------------------------------------------------------
# Project include file
# Copyright (c) 2009-2018 Avinger, Inc
# -----------------------------------------------------------
# Header files
HEADERS += \
    ../buildflags.h \
    ../version.h \
    Backend/dspgpu.h \
    Backend/initialization.h \
    Backend/daqSettings.h \
    Backend/laser.h \
    Backend/videoencoder.h \
    Include/dataPlotWidget.h \
    Include/octFile.h \
    Include/scanLine.h \
    Frontend/Screens/docscreen.h \
    Frontend/Screens/frontend.h \
    Frontend/Utility/captureListModel.h \
    Frontend/Utility/clipListModel.h \
    Frontend/Utility/userSettings.h \
    Frontend/Widgets/advancedview.h \
    Frontend/Widgets/livescene.h \
    Frontend/Widgets/liveview.h \
    Frontend/Widgets/sectoritem.h \
    Frontend/Widgets/waterfall.h \
    Frontend/Utility/capturemachine.h \
    Frontend/Widgets/capturewidget.h \
    Frontend/Widgets/gridOverlay.h \
    Frontend/Widgets/lagwizard.h \
    Frontend/Widgets/rotatableLabel.h \
    Frontend/Widgets/caseinfowizard.h \
    Frontend/Widgets/capturelistview.h \
    Frontend/Widgets/cliplistview.h \
    Include/eventDataLog.h \
    Frontend/Widgets/devicewizard.h \
    Frontend/Widgets/deviceselectwizardpage.h \
    Frontend/Widgets/deviceconfirmwizardpage.h \
    Include/qtsingleapplication.h \
    Frontend/Utility/daqDataConsumer.h \
    Frontend/Widgets/caseinfowizardpage.h \
    Frontend/Widgets/notificationwidget.h \
    Frontend/Utility/sessiondatabase.h \
    Frontend/Widgets/avDisplayControls.h \
    Frontend/Widgets/viewoptions.h \
    Frontend/Utility/directionTracker.h \
    ../../Common/Include/rotaryAverage.h \
    ../../Common/Include/unwindMachine.h \
    ../../Common/Include/trigLookupTable.h \
    ../../Common/Include/Integrator.h \
    ../../Common/Include/defaults.h \
    ../../Common/Include/session.h \
    ../../Common/Include/logger.h \
    ../../Common/Include/deviceSettings.h \
    ../../Common/Include/avTimeEdit.h \
    ../../Common/Include/styledmessagebox.h \
    ../../Common/Include/sawFile.h \
    ../../Common/Include/keys.h \
    ../../Common/Include/util.h \
    ../../Common/Include/fileUtil.h \
    ../../Common/Include/windowmanager.h \
    Backend/ioController.h \
    Backend/evoa.h \
    Backend/powerdistributionboard.h \
    Backend/dsp.h \
    Backend/depthsetting.h \
    Backend/sledsupport.h \
    Frontend/Widgets/curveswidget.h \
    Frontend/Widgets/curvesdialog.h \
    Frontend/Widgets/areameasurementoverlay.h \
    Frontend/Widgets/levelgauge.h \
    ../../Common/GUI/transport.h \
    Frontend/Widgets/annotateoverlay.h \
    $$PWD/../../Common/GUI/scrubbar.h \
    Frontend/Screens/auxmonitor.h \
    ../../Common/Include/keyboardinputcontext.h \
    ../../Common/Include/keyboard.h \
    ../../Common/Include/backgroundmask.h \
    Backend/ftd2xx.h \
    $$PWD/Backend/daqfactory.h \
    $$PWD/Backend/idaq.h \
    $$PWD/Frontend/Widgets/engineeringdialog.h \
    $$PWD/Frontend/Widgets/engineeringcontroller.h \
    $$PWD/Frontend/Widgets/enginneringmodel.h \
    $$PWD/Backend/playbackmanager.h \
    $$PWD/Backend/playbackmanager.h \
    $$PWD/Backend/filedaq.h \
    $$PWD/Backend/theglobals.h \
    $$PWD/Backend/signalmanager.h

# Source files
SOURCES += \
    Backend/dspgpu.cpp \
    Backend/initialization.cpp \
    Backend/daqSettings.cpp \
    Backend/laser.cpp \
    Backend/videoencoder.cpp \
    Utility/octFile.cpp \
    Frontend/Screens/docscreen.cpp \
    Frontend/Screens/frontend.cpp \
    Frontend/Utility/captureListModel.cpp \
    Frontend/Utility/clipListModel.cpp \
    Frontend/Utility/userSettings.cpp \
    Frontend/Widgets/advancedview.cpp \
    Frontend/Widgets/dataPlotWidget.cpp \
    Frontend/Widgets/livescene.cpp \
    Frontend/Widgets/liveview.cpp \
    Frontend/Widgets/sectoritem.cpp \
    Frontend/Widgets/waterfall.cpp \
    main.cpp \
    Frontend/Utility/capturemachine.cpp \
    Frontend/Widgets/capturewidget.cpp \
    Frontend/Widgets/gridOverlay.cpp \
    Frontend/Widgets/lagwizard.cpp \
    Frontend/Widgets/rotatableLabel.cpp \
    Frontend/Widgets/caseinfowizard.cpp \
    Frontend/Widgets/capturelistview.cpp \
    Frontend/Widgets/cliplistview.cpp \
    Frontend/Utility/daqDataConsumer.cpp \
    Utility/eventDataLog.cpp \
    Frontend/Widgets/devicewizard.cpp \
    Frontend/Widgets/deviceselectwizardpage.cpp \
    Frontend/Widgets/deviceconfirmwizardpage.cpp \
    Frontend/Widgets/caseinfowizardpage.cpp \
    Frontend/Widgets/notificationwidget.cpp \
    Frontend/Widgets/viewoptions.cpp \
    Utility/qtsingleapplication.cpp \
    Frontend/Utility/sessiondatabase.cpp \
    Frontend/Widgets/avDisplayControls.cpp \
    Frontend/Utility/directionTracker.cpp \
    ../../Common/Utility/rotaryAverage.cpp \
    ../../Common/Utility/Integrator.cpp \
    ../../Common/Utility/trigLookupTable.cpp \
    ../../Common/Utility/unwindMachine.cpp \
    ../../Common/Utility/session.cpp \
    ../../Common/Utility/logger.cpp \
    ../../Common/Utility/deviceSettings.cpp \
    ../../Common/Utility/avTimeEdit.cpp \
    ../../Common/GUI/styledmessagebox.cpp \
    ../../Common/Utility/sawFile.cpp \
    ../../Common/Utility/util.cpp \
    ../../Common/Utility/keys.cpp \
    ../../Common/Utility/fileUtil.cpp \
    Backend/ioController.cpp \
    Backend/evoa.cpp \ 
    Backend/powerdistributionboard.cpp \
    Backend/dsp.cpp \
    Backend/depthsetting.cpp \
    Backend/sledsupport.cpp \
    Frontend/Widgets/curveswidget.cpp \
    Frontend/Widgets/curvesdialog.cpp \
    Frontend/Widgets/areameasurementoverlay.cpp \
    Frontend/Widgets/levelgauge.cpp \
    Frontend/Widgets/annotateoverlay.cpp \
    ../../Common/GUI/transport.cpp \
    ../../Common/GUI/scrubbar.cpp \
    Frontend/Screens/auxmonitor.cpp \
    ../../Common/GUI/touchKeyboard/keyboardinputcontext.cpp \
    ../../Common/GUI/touchKeyboard/keyboard.cpp \
    ../../Common/GUI/backgroundmask.cpp \
    ../../Common/GUI/windowmanager.cpp \
    $$PWD/Backend/daqfactory.cpp \
    $$PWD/Frontend/Widgets/engineeringdialog.cpp \
    $$PWD/Frontend/Widgets/engineeringcontroller.cpp \
    $$PWD/Frontend/Widgets/enginneringmodel.cpp \
    $$PWD/Backend/playbackmanager.cpp \
    $$PWD/Backend/filedaq.cpp \
    $$PWD/Backend/theglobals.cpp \
    $$PWD/Backend/signalmanager.cpp

win32:SOURCES += Utility/qtsingleapplication_win.cpp
unix:SOURCES += Utility/qtsingleapplication_x11.cpp

# Forms
FORMS += Frontend/Screens/docscreen.ui \
    Frontend/Screens/frontend.ui \
    Frontend/Widgets/advancedview.ui \
    Frontend/Widgets/capturewidget.ui \
    Frontend/Widgets/lagwizard.ui \
    Frontend/Widgets/caseinfowizard.ui \
    Frontend/Widgets/devicewizard.ui \
    Frontend/Widgets/deviceselectwizardpage.ui \
    Frontend/Widgets/deviceconfirmwizardpage.ui \
    Frontend/Widgets/caseinfowizardpage.ui \
    Frontend/Widgets/viewoptions.ui \
    ../../Common/GUI/styledmessagebox.ui \ 
    Frontend/Widgets/curvesdialog.ui \
    Frontend/Widgets/levelgauge.ui \
    $$PWD/../../Common/GUI/transport.ui \
    Frontend/Screens/auxmonitor.ui \
    $$PWD/Frontend/Widgets/engineeringdialog.ui

# Resource file(s)
RESOURCES += ./octConsole.qrc \
    ../../Common/Resources/common.qrc

# Windows-specific resources
win32:RC_FILE += octConsole.rc

OTHER_FILES += \
    Backend/OpenCL/postProc.cl \
    Backend/OpenCL/warp.cl

DISTFILES += \
    $$PWD/Backend/OpenCL/bandc.cl
