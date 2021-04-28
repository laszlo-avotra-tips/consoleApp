#include "powerUpDiagnostics.h"
#include <QApplication>
#include <QDirIterator>
#include <QDate>

#include "initialization.h"
#include "sawFile.h"
#include "defaults.h"
#include "keys.h"
#include "logger.h"
#include "Utility/userSettings.h"

/*
 * PowerUpDiagnostics constructor
 *
 * Performs all necessary tests to make sure the system is ready to run; if
 * any tests fail, the application may report the failure and shuts it down.
 */
PowerUpDiagnostics::PowerUpDiagnostics(QObject *parent) : OctSystemDiagnostics(parent)
{
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_INIT_KEY_CHECK, "OCT_INIT_KEY_CHECK");
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_INIT_IF_BOARD_COMPATIBILITY_CHECK, "OCT_INIT_IF_BOARD_COMPATIBILITY_CHECK");
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_INIT_AC_POWER_CHECK, "OCT_INIT_AC_POWER_CHECK", 5);
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_INIT_SYSTEM_DATE_CHECK, "OCT_INIT_SYSTEM_DATE_CHECK");
    diagnosticModule = "Power up diagnostic check";
}

/*
 * verifyAssets
 *
 * Performs assets verification of software components in the system; If
 * there is any incompatibilities in the components, flag it as critical error.
 * Application must terminate reporting this as key check failure
 */
int PowerUpDiagnostics::verifyAssets() {
    int result = getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_INIT_KEY_CHECK);

    if (result == -1) {
        currentRetriesAttempted = 0;
    }

    if (result != 1) {
        const QString applicationDirPath = QCoreApplication::applicationDirPath();
        QString keyFilePath = SystemDir + "/" + L300KeyFile;

        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_INIT_KEY_CHECK;

        Keys binaryKeysReadList( keyFilePath, Keys::ReadOnly );
        if (binaryKeysReadList.init(applicationDirPath)) {
            LOG1("Key check succeeded");
            return 1; // Success
        }

        statusMessage = tr( "The executable file has been tampered with or\n" \
                            " is not the original version that was installed." );
        LOG(ERROR, "Keys check failed!");
        return 0; // Failure
    }

    return result;
}

/*
 * performInterfaceBoardCompatibilityCheck
 *
 * Performs compatibility check for the interface board; If there is any
 * incompatibilities detected in the interface board, application should flag
 * it as a critical error. Application must terminate once this condition is detected
 */
int PowerUpDiagnostics::performInterfaceBoardCompatibilityCheck() {
    int result = getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_INIT_IF_BOARD_COMPATIBILITY_CHECK);

    if (result == -1) {
        currentRetriesAttempted = 0;
    }

    if (result != 1) {
        userSettings &settings = userSettings::Instance();
        float minimumHardwareVersion = settings.getInterface_hw_version().toFloat();
        float minimumFirmwareVersion = settings.getInterface_firmware_version().toFloat();
        LOG2(minimumHardwareVersion, minimumFirmwareVersion)
        auto interfaceSupport = InterfaceSupport::getInstance(true); //initialize FTDI and also reset the interface board
        QString msg("initialize FTDI and also reset the interface board");
        LOG1(msg);
        float hardwareVersion = 0.0;
        float firmwareVersion = 0.0;

        if (interfaceSupport) {
            hardwareVersion = interfaceSupport->getHardwareVersion();
            firmwareVersion = interfaceSupport->getFirmwareVersion();
            LOG2(hardwareVersion, firmwareVersion);
        } else {
            // InterfaceSupport cannot be instatiated possibly due to FTDI error.
            // Flag this as test not run
            return -1;
        }

        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_INIT_IF_BOARD_COMPATIBILITY_CHECK;

        if (minimumHardwareVersion > hardwareVersion) {
            LOG2(minimumHardwareVersion, hardwareVersion);
            statusMessage = tr( "Interface board hardware not compatible" );
            LOG(ERROR, "Interface board hardware not compatible");
            return 0; // Failure
        } else if (minimumFirmwareVersion > firmwareVersion) {
            LOG2(minimumFirmwareVersion, firmwareVersion);
            statusMessage = tr( "Interface board firmware not compatible" );
            LOG(ERROR, "Interface board firmware not compatible");
            return 0; // Failure
        }

        return 1; // Success
    }

    return result;
}

/*
 * performACPowerCheck
 *
 * Performs AC power check; If application is running without AC power, then it should flag
 * it as a critical error. Application must terminate once this condition is detected
 */
/*
 * performSystemDateCheck
 *
 * Performs system date check; If the date is older than Jan 1st 2020, then report
 * the user about a possible CMS battery failure in the system as a warning. Application
 * may continue to run in such event with a display prompt to the user about the condition.
 */
int PowerUpDiagnostics::performSystemDateCheck() {
    int result = getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_INIT_SYSTEM_DATE_CHECK);

    if (result == -1) {
        currentRetriesAttempted = 0;
    }

    if (result != 1) {
        QDate currDate = QDate::currentDate();
        QDate allowedOlderDate(2020, 1, 1);

        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_INIT_SYSTEM_DATE_CHECK;

        if (currDate > allowedOlderDate) {
            return 1; // Success
        }

        statusMessage = tr( "Older date detected in the system. Possibly due to CMOS error" );
        LOG(WARNING, "Older date detected in the system. Possibly due to CMOS error");
        return 0; // Failure
    }

    return result;
}

bool PowerUpDiagnostics::   performDiagnostics(bool checkAll) {
    if (checkAll) {
        resetResultsTable();
    }

    int checkResult = verifyAssets();
    LOG1(checkResult);

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_INIT_KEY_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    checkResult = performInterfaceBoardCompatibilityCheck();
    LOG1(checkResult);

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_INIT_IF_BOARD_COMPATIBILITY_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    checkResult = performACPowerCheck();
    LOG1(checkResult);

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_INIT_AC_POWER_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    checkResult = performSystemDateCheck();
    LOG1(checkResult);

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_INIT_SYSTEM_DATE_CHECK,
                                            "Warning", checkResult)) {
        return false;
    }

    emit hideMessageBox();
    return true;
}

int PowerUpDiagnostics::areKeysVerified() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_INIT_KEY_CHECK);
}

int PowerUpDiagnostics::isInterfaceBoardChecked() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_INIT_IF_BOARD_COMPATIBILITY_CHECK);
}

int PowerUpDiagnostics::isSystemDateChecked() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_INIT_SYSTEM_DATE_CHECK);
}
