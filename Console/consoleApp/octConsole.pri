# -----------------------------------------------------------
# Project include file
# Copyright (c) 2009-2018 Avinger, Inc
# -----------------------------------------------------------
# Header files
HEADERS += \
    $$PWD/Backend/AxsunCommonEnums.h \
    $$PWD/Backend/AxsunOCTCapture.h \
    $$PWD/Backend/AxsunOCTControl_LW_C.h \
    $$PWD/Backend/backend.h \
    $$PWD/Backend/bandc.h \
    $$PWD/Backend/daq.h \
    $$PWD/Backend/ikernelfunction.h \
    $$PWD/Backend/imagedescriptor.h \
    $$PWD/Backend/kernelfunctionbase.h \
    $$PWD/Backend/postfft.h \
    $$PWD/Backend/producer.h \
    $$PWD/Backend/scanconversion.h \
    $$PWD/Backend/signalprocessingfactory.h \
    $$PWD/Backend/warp.h \
    $$PWD/Backend/warpbc.h \
    $$PWD/Frontend/Utility/dialogFactory.h \
    $$PWD/Frontend/Utility/screenFactory.h \
    $$PWD/Frontend/Utility/widgetcontainer.h \
    $$PWD/Frontend/Widgets/DisplayOptionsModel.h \
    $$PWD/Frontend/Widgets/caseInformationDialog.h \
    $$PWD/Frontend/Widgets/caseInformationModel.h \
    $$PWD/Frontend/Widgets/consoleKeyboard.h \
    $$PWD/Frontend/Widgets/consoleLabel.h \
    $$PWD/Frontend/Widgets/consoleLineEdit.h \
    $$PWD/Frontend/Widgets/deviceDelegate.h \
    $$PWD/Frontend/Widgets/deviceDisplayModel.h \
    $$PWD/Frontend/Widgets/deviceListModel.h \
    $$PWD/Frontend/Widgets/deviceSelectDialog.h \
    $$PWD/Frontend/Widgets/displayOptionsDialog.h \
    $$PWD/Frontend/Widgets/emptyDialog.h \
    $$PWD/Frontend/Widgets/mainScreen.h \
    $$PWD/Frontend/Widgets/reviewAndSettingsDialog.h \
    $$PWD/Frontend/Widgets/screenNavigator.h \
    $$PWD/Frontend/Widgets/selectDialog.h \
    $$PWD/Frontend/Widgets/startScreen.h \
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
    Frontend/Screens/frontend.h \
    Frontend/Utility/captureListModel.h \
    Frontend/Utility/clipListModel.h \
    Frontend/Utility/userSettings.h \
    Frontend/Widgets/livescene.h \
    Frontend/Widgets/liveview.h \
    Frontend/Widgets/sectoritem.h \
    Frontend/Utility/capturemachine.h \
    Frontend/Widgets/rotatableLabel.h \
    Include/eventDataLog.h \
    Include/qtsingleapplication.h \
    Frontend/Utility/daqDataConsumer.h \
    Frontend/Widgets/notificationwidget.h \
    Frontend/Utility/sessiondatabase.h \
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
    Frontend/Widgets/areameasurementoverlay.h \
    Frontend/Widgets/levelgauge.h \
    ../../Common/GUI/transport.h \
    Frontend/Widgets/annotateoverlay.h \
    $$PWD/../../Common/GUI/scrubbar.h \
    ../../Common/Include/keyboardinputcontext.h \
    ../../Common/Include/keyboard.h \
    ../../Common/Include/backgroundmask.h \
    Backend/ftd2xx.h \
    $$PWD/Backend/daqfactory.h \
    $$PWD/Backend/idaq.h \
    $$PWD/Backend/playbackmanager.h \
    $$PWD/Backend/filedaq.h \
    $$PWD/Backend/signalmanager.h \
    $$PWD/Backend/signalmodel.h

# Source files
SOURCES += \
    $$PWD/Backend/backend.cpp \
    $$PWD/Backend/bandc.cpp \
    $$PWD/Backend/daq.cpp \
    $$PWD/Backend/imagedescriptor.cpp \
    $$PWD/Backend/kernelfunctionbase.cpp \
    $$PWD/Backend/postfft.cpp \
    $$PWD/Backend/producer.cpp \
    $$PWD/Backend/scanconversion.cpp \
    $$PWD/Backend/signalprocessingfactory.cpp \
    $$PWD/Backend/warp.cpp \
    $$PWD/Backend/warpbc.cpp \
    $$PWD/Frontend/Utility/dialogFactory.cpp \
    $$PWD/Frontend/Utility/screenFactory.cpp \
    $$PWD/Frontend/Utility/widgetcontainer.cpp \
    $$PWD/Frontend/Widgets/DisplayOptionsModel.cpp \
    $$PWD/Frontend/Widgets/caseInformationDialog.cpp \
    $$PWD/Frontend/Widgets/caseInformationModel.cpp \
    $$PWD/Frontend/Widgets/consoleKeyboard.cpp \
    $$PWD/Frontend/Widgets/consoleLabel.cpp \
    $$PWD/Frontend/Widgets/consoleLineEdit.cpp \
    $$PWD/Frontend/Widgets/deviceDelegate.cpp \
    $$PWD/Frontend/Widgets/deviceDisplayModel.cpp \
    $$PWD/Frontend/Widgets/deviceListModel.cpp \
    $$PWD/Frontend/Widgets/deviceSelectDialog.cpp \
    $$PWD/Frontend/Widgets/displayOptionsDialog.cpp \
    $$PWD/Frontend/Widgets/emptyDialog.cpp \
    $$PWD/Frontend/Widgets/mainScreen.cpp \
    $$PWD/Frontend/Widgets/reviewAndSettingsDialog.cpp \
    $$PWD/Frontend/Widgets/screenNavigator.cpp \
    $$PWD/Frontend/Widgets/selectDialog.cpp \
    $$PWD/Frontend/Widgets/startScreen.cpp \
    $$PWD/main.cpp \
    Backend/dspgpu.cpp \
    Backend/initialization.cpp \
    Backend/daqSettings.cpp \
    Backend/laser.cpp \
    Backend/videoencoder.cpp \
    Utility/octFile.cpp \
    Frontend/Screens/frontend.cpp \
    Frontend/Utility/captureListModel.cpp \
    Frontend/Utility/clipListModel.cpp \
    Frontend/Utility/userSettings.cpp \
    Frontend/Widgets/livescene.cpp \
    Frontend/Widgets/liveview.cpp \
    Frontend/Widgets/sectoritem.cpp \
    Frontend/Utility/capturemachine.cpp \
    Frontend/Widgets/rotatableLabel.cpp \
    Frontend/Utility/daqDataConsumer.cpp \
    Utility/eventDataLog.cpp \
    Frontend/Widgets/notificationwidget.cpp \
    Frontend/Widgets/viewoptions.cpp \
    Utility/qtsingleapplication.cpp \
    Frontend/Utility/sessiondatabase.cpp \
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
    Frontend/Widgets/areameasurementoverlay.cpp \
    Frontend/Widgets/levelgauge.cpp \
    Frontend/Widgets/annotateoverlay.cpp \
    ../../Common/GUI/transport.cpp \
    ../../Common/GUI/scrubbar.cpp \
    ../../Common/GUI/touchKeyboard/keyboardinputcontext.cpp \
    ../../Common/GUI/touchKeyboard/keyboard.cpp \
    ../../Common/GUI/backgroundmask.cpp \
    ../../Common/GUI/windowmanager.cpp \
    $$PWD/Backend/daqfactory.cpp \
    $$PWD/Backend/playbackmanager.cpp \
    $$PWD/Backend/filedaq.cpp \
    $$PWD/Backend/signalmanager.cpp \
    $$PWD/Backend/signalmodel.cpp

win32:SOURCES += Utility/qtsingleapplication_win.cpp
unix:SOURCES += Utility/qtsingleapplication_x11.cpp

# Forms
FORMS += \
    $$PWD/Frontend/Widgets/caseInformationDialog.ui \
    $$PWD/Frontend/Widgets/consoleKeyboard.ui \
    $$PWD/Frontend/Widgets/deviceSelectDialog.ui \
    $$PWD/Frontend/Widgets/displayOptionsDialog.ui \
    $$PWD/Frontend/Widgets/emptyDialog.ui \
    $$PWD/Frontend/Widgets/mainScreen.ui \
    $$PWD/Frontend/Widgets/reviewAndSettingsDialog.ui \
    $$PWD/Frontend/Widgets/screenNavigator.ui \
    $$PWD/Frontend/Widgets/selectDialog.ui \
    $$PWD/Frontend/Widgets/startScreen.ui \
    Frontend/Screens/frontend.ui \
    Frontend/Widgets/viewoptions.ui \
    ../../Common/GUI/styledmessagebox.ui \ 
    Frontend/Widgets/levelgauge.ui \
    $$PWD/../../Common/GUI/transport.ui

# Resource file(s)
RESOURCES += ./octConsole.qrc \
    ../../Common/Resources/common.qrc

# Windows-specific resources
win32:RC_FILE += octConsole.rc
