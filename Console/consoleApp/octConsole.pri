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
    $$PWD/Backend/daq.h \
    $$PWD/Backend/displayManager.h \
    $$PWD/Backend/fullCaseRecorder.h \
    $$PWD/Backend/imagedescriptor.h \
    $$PWD/Backend/scanconversion.h \
    $$PWD/Frontend/Utility/ScreenCapture.hpp \
    $$PWD/Frontend/Utility/caseInfoDatabase.h \
    $$PWD/Frontend/Utility/clipListModel.h \
    $$PWD/Frontend/Utility/concatenateVideo.h \
    $$PWD/Frontend/Utility/dialogFactory.h \
    $$PWD/Frontend/Utility/octFrameRecorder.h \
    $$PWD/Frontend/Utility/screenFactory.h \
    $$PWD/Frontend/Utility/widgetcontainer.h \
    $$PWD/Frontend/Widgets/DisplayOptionsModel.h \
    $$PWD/Frontend/Widgets/activeLabel.h \
    $$PWD/Frontend/Widgets/captureItemDelegate.h \
    $$PWD/Frontend/Widgets/capturelistview.h \
    $$PWD/Frontend/Widgets/caseInformationDialog.h \
    $$PWD/Frontend/Widgets/caseInformationModel.h \
    $$PWD/Frontend/Widgets/caseReviewScreen.h \
    $$PWD/Frontend/Widgets/clipItemDelegate.h \
    $$PWD/Frontend/Widgets/cliplistview.h \
    $$PWD/Frontend/Widgets/consoleKeyboard.h \
    $$PWD/Frontend/Widgets/consoleLabel.h \
    $$PWD/Frontend/Widgets/consoleLineEdit.h \
    $$PWD/Frontend/Widgets/deviceDelegate.h \
    $$PWD/Frontend/Widgets/deviceDisplayModel.h \
    $$PWD/Frontend/Widgets/deviceListModel.h \
    $$PWD/Frontend/Widgets/deviceSelectDialog.h \
    $$PWD/Frontend/Widgets/displayOptionsDialog.h \
    $$PWD/Frontend/Widgets/emptyDialog.h \
    $$PWD/Frontend/Widgets/formDisk.h \
    $$PWD/Frontend/Widgets/formPmCaseReview.h \
    $$PWD/Frontend/Widgets/formPmLogo.h \
    $$PWD/Frontend/Widgets/formSecondMonitor.h \
    $$PWD/Frontend/Widgets/iRotationIndicator.h \
    $$PWD/Frontend/Widgets/mainScreen.h \
    $$PWD/Frontend/Widgets/opaqueScreen.h \
    $$PWD/Frontend/Widgets/preferencesDialog.h \
    $$PWD/Frontend/Widgets/reviewAndSettingsDialog.h \
    $$PWD/Frontend/Widgets/rotationIndicatorFactory.h \
    $$PWD/Frontend/Widgets/rotationIndicatorOverlay.h \
    $$PWD/Frontend/Widgets/rotationIndicatorOverlay2.h \
    $$PWD/Frontend/Widgets/screenNavigator.h \
    $$PWD/Frontend/Widgets/selectDialog.h \
    $$PWD/Frontend/Widgets/startScreen.h \
    $$PWD/Frontend/Widgets/videoplayer.h \
    ../version.h \
    Backend/initialization.h \
    Include/dataPlotWidget.h \
    Include/octFile.h \
    Include/scanLine.h \
    Frontend/Utility/captureListModel.h \
    Frontend/Utility/userSettings.h \
    Frontend/Widgets/livescene.h \
    Frontend/Widgets/sectoritem.h \
    Frontend/Utility/capturemachine.h \
    Include/eventDataLog.h \
    Include/qtsingleapplication.h \
    Frontend/Utility/sessiondatabase.h \
    Frontend/Utility/directionTracker.h \
    ../../Common/Include/rotaryAverage.h \
    ../../Common/Include/unwindMachine.h \
    ../../Common/Include/trigLookupTable.h \
    ../../Common/Include/Integrator.h \
    ../../Common/Include/defaults.h \
    ../../Common/Include/logger.h \
    ../../Common/Include/deviceSettings.h \
    ../../Common/Include/styledmessagebox.h \
    ../../Common/Include/sawFile.h \
    ../../Common/Include/keys.h \
    ../../Common/Include/util.h \
    Backend/dsp.h \
    Backend/depthsetting.h \
    Backend/sledsupport.h \
    Frontend/Widgets/areameasurementoverlay.h \
    Frontend/Widgets/annotateoverlay.h \
    ../../Common/Include/backgroundmask.h \
    Backend/ftd2xx.h \
    $$PWD/Backend/daqfactory.h \
    $$PWD/Backend/idaq.h \
    $$PWD/Backend/signalmodel.h

# Source files
SOURCES += \
    $$PWD/Backend/backend.cpp \
    $$PWD/Backend/daq.cpp \
    $$PWD/Backend/displayManager.cpp \
    $$PWD/Backend/fullCaseRecorder.cpp \
    $$PWD/Backend/imagedescriptor.cpp \
    $$PWD/Backend/scanconversion.cpp \
    $$PWD/Frontend/Utility/caseInfoDatabase.cpp \
    $$PWD/Frontend/Utility/clipListModel.cpp \
    $$PWD/Frontend/Utility/concatenateVideo.cpp \
    $$PWD/Frontend/Utility/dialogFactory.cpp \
    $$PWD/Frontend/Utility/octFrameRecorder.cpp \
    $$PWD/Frontend/Utility/screenFactory.cpp \
    $$PWD/Frontend/Utility/widgetcontainer.cpp \
    $$PWD/Frontend/Widgets/DisplayOptionsModel.cpp \
    $$PWD/Frontend/Widgets/activeLabel.cpp \
    $$PWD/Frontend/Widgets/captureItemDelegate.cpp \
    $$PWD/Frontend/Widgets/capturelistview.cpp \
    $$PWD/Frontend/Widgets/caseInformationDialog.cpp \
    $$PWD/Frontend/Widgets/caseInformationModel.cpp \
    $$PWD/Frontend/Widgets/caseReviewScreen.cpp \
    $$PWD/Frontend/Widgets/clipItemDelegate.cpp \
    $$PWD/Frontend/Widgets/cliplistview.cpp \
    $$PWD/Frontend/Widgets/consoleKeyboard.cpp \
    $$PWD/Frontend/Widgets/consoleLabel.cpp \
    $$PWD/Frontend/Widgets/consoleLineEdit.cpp \
    $$PWD/Frontend/Widgets/deviceDelegate.cpp \
    $$PWD/Frontend/Widgets/deviceDisplayModel.cpp \
    $$PWD/Frontend/Widgets/deviceListModel.cpp \
    $$PWD/Frontend/Widgets/deviceSelectDialog.cpp \
    $$PWD/Frontend/Widgets/displayOptionsDialog.cpp \
    $$PWD/Frontend/Widgets/emptyDialog.cpp \
    $$PWD/Frontend/Widgets/formDisk.cpp \
    $$PWD/Frontend/Widgets/formPmCaseReview.cpp \
    $$PWD/Frontend/Widgets/formPmLogo.cpp \
    $$PWD/Frontend/Widgets/formSecondMonitor.cpp \
    $$PWD/Frontend/Widgets/mainScreen.cpp \
    $$PWD/Frontend/Widgets/opaqueScreen.cpp \
    $$PWD/Frontend/Widgets/preferencesDialog.cpp \
    $$PWD/Frontend/Widgets/reviewAndSettingsDialog.cpp \
    $$PWD/Frontend/Widgets/rotationIndicatorFactory.cpp \
    $$PWD/Frontend/Widgets/rotationIndicatorOverlay.cpp \
    $$PWD/Frontend/Widgets/rotationIndicatorOverlay2.cpp \
    $$PWD/Frontend/Widgets/screenNavigator.cpp \
    $$PWD/Frontend/Widgets/selectDialog.cpp \
    $$PWD/Frontend/Widgets/startScreen.cpp \
    $$PWD/Frontend/Widgets/videoplayer.cpp \
    $$PWD/main.cpp \
    Backend/initialization.cpp \
    Utility/octFile.cpp \
    Frontend/Utility/captureListModel.cpp \
    Frontend/Utility/userSettings.cpp \
    Frontend/Widgets/livescene.cpp \
    Frontend/Widgets/sectoritem.cpp \
    Frontend/Utility/capturemachine.cpp \
    Utility/eventDataLog.cpp \
    Utility/qtsingleapplication.cpp \
    Frontend/Utility/sessiondatabase.cpp \
    Frontend/Utility/directionTracker.cpp \
    ../../Common/Utility/rotaryAverage.cpp \
    ../../Common/Utility/Integrator.cpp \
    ../../Common/Utility/trigLookupTable.cpp \
    ../../Common/Utility/unwindMachine.cpp \
    ../../Common/Utility/logger.cpp \
    ../../Common/Utility/deviceSettings.cpp \
    ../../Common/GUI/styledmessagebox.cpp \
    ../../Common/Utility/sawFile.cpp \
    ../../Common/Utility/util.cpp \
    ../../Common/Utility/keys.cpp \
    Backend/dsp.cpp \
    Backend/depthsetting.cpp \
    Backend/sledsupport.cpp \
    Frontend/Widgets/areameasurementoverlay.cpp \
    Frontend/Widgets/annotateoverlay.cpp \
    ../../Common/GUI/backgroundmask.cpp \
    $$PWD/Backend/daqfactory.cpp \
    $$PWD/Backend/signalmodel.cpp

win32:SOURCES += Utility/qtsingleapplication_win.cpp
unix:SOURCES += Utility/qtsingleapplication_x11.cpp

# Forms
FORMS += \
    $$PWD/Frontend/Widgets/caseInformationDialog.ui \
    $$PWD/Frontend/Widgets/caseReviewScreen.ui \
    $$PWD/Frontend/Widgets/consoleKeyboard.ui \
    $$PWD/Frontend/Widgets/deviceSelectDialog.ui \
    $$PWD/Frontend/Widgets/displayOptionsDialog.ui \
    $$PWD/Frontend/Widgets/emptyDialog.ui \
    $$PWD/Frontend/Widgets/formDisk.ui \
    $$PWD/Frontend/Widgets/formPmCaseReview.ui \
    $$PWD/Frontend/Widgets/formPmLogo.ui \
    $$PWD/Frontend/Widgets/formSecondMonitor.ui \
    $$PWD/Frontend/Widgets/mainScreen.ui \
    $$PWD/Frontend/Widgets/opaqueScreen.ui \
    $$PWD/Frontend/Widgets/preferencesDialog.ui \
    $$PWD/Frontend/Widgets/reviewAndSettingsDialog.ui \
    $$PWD/Frontend/Widgets/screenNavigator.ui \
    $$PWD/Frontend/Widgets/selectDialog.ui \
    $$PWD/Frontend/Widgets/startScreen.ui \
    ../../Common/GUI/styledmessagebox.ui  

# Resource file(s)
RESOURCES += ./octConsole.qrc \
    ../../Common/Resources/common.qrc

# Windows-specific resources
win32:RC_FILE += octConsole.rc
