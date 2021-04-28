#include "startupdiagnostics.h"
#include "logger.h"
#include "Utility/userSettings.h"

StartUpDiagnostics::StartUpDiagnostics(QObject *parent) : OctSystemDiagnostics(parent) {
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_INIT_AC_POWER_CHECK, "OCT_INIT_AC_POWER_CHECK", 5);
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_START_OCT_POWER_UP_CHECK, "OCT_START_OCT_POWER_UP_CHECK");
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_START_OCT_SLED_ON_CHECK, "OCT_START_OCT_SLED_ON_CHECK");
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_START_SLED_COMPATIBILITY_CHECK, "OCT_INIT_SYSTEM_DATE_CHECK");
    diagnosticModule = "Start up diagnostic check";
}

int StartUpDiagnostics::performOctPowerUpChecks() {
    auto interfaceSupport = InterfaceSupport::getInstance();

    if (interfaceSupport) {
        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_START_OCT_POWER_UP_CHECK;
        bool operationResult = interfaceSupport->turnOnOffACPowerToOCT(true);

        if (operationResult) {
            LOG(INFO, "Successfully turned on AC power to OCT");
            operationResult = interfaceSupport->setVOAMode(true);

            if (operationResult) {
                LOG( INFO, "Successfully set VOA to transparent mode");
                int voaVal = interfaceSupport->getVOASettings();

                if (voaVal == -1) {
                    // InterfaceSupport cannot fetch VOA value from the board.
                    // Flag this as error
                    statusMessage = tr( "Failed to get VOA settings");
                    return 0;
                } else {
                    LOG( INFO, "Successfully obtained VOA seetings from interface board");
                }
            } else {
                // InterfaceSupport cannot turn on AC power to OCT.
                // Flag this as error
                statusMessage = tr( "Failed to set VOA to transparent mode");
                return 0;
            }
        } else {
            // InterfaceSupport cannot turn on AC power to OCT.
            // Flag this as error
            statusMessage = tr( "Failed to turn on AC power to OCT");
            return 0;
        }
    } else {
        // InterfaceSupport cannot be instatiated possibly due to FTDI error.
        // Flag this as test not run
        return -1;
    }
    return 1; // We passed all checks
}

int StartUpDiagnostics::turnOnSled() {
    auto interfaceSupport = InterfaceSupport::getInstance();

    if (interfaceSupport) {
        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_START_OCT_SLED_ON_CHECK;
        bool operationResult = interfaceSupport->turnOnOffSled(true);

        if (operationResult) {
            LOG( INFO, "Successfully turned on Sled");
        } else {
            LOG( ERROR, "Failed to turn on Sled");
            statusMessage = tr( "Failed to turn on Sled");
            return 0;
        }
        operationResult = interfaceSupport->turnOnOffLaser(false);

        if (operationResult) {
            LOG( INFO, "Successfully turned on Laser");
        } else {
            LOG( ERROR, "Failed to turn on Laser");
            statusMessage = tr( "Failed to turn on Laser");
            return 0;
        }
    } else {
        // InterfaceSupport cannot be instatiated possibly due to FTDI error.
        // Flag this as test not run
        return -1;
    }

    return 1; // We passed the check
}

int StartUpDiagnostics::performSledCompatibilityCheck() {
    auto interfaceSupport = InterfaceSupport::getInstance();

    if (interfaceSupport) {
        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_START_SLED_COMPATIBILITY_CHECK;
        userSettings &settings = userSettings::Instance();
        float minimumSledVersion = settings.getSled_firmware_version().toFloat();

        float detectedSledVersion = interfaceSupport->getSledFirmwareVersion();

        if (minimumSledVersion > detectedSledVersion) {
            LOG2(minimumSledVersion, detectedSledVersion);
            statusMessage = tr( "Sled compatibility check failed" );
            LOG(ERROR, "Sled compatibility check failed");
            return 0; // Failure
        }
    } else {
        // InterfaceSupport cannot be instatiated possibly due to FTDI error.
        // Flag this as test not run
        return -1;
    }

    return 1; // Success
}

bool StartUpDiagnostics::performDiagnostics(bool checkAll) {
    if (checkAll) {
        resetResultsTable();
    }

    bool checkResult = performACPowerCheck();

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_INIT_AC_POWER_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    checkResult = performOctPowerUpChecks();

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_START_OCT_POWER_UP_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    checkResult = turnOnSled();

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_START_OCT_SLED_ON_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    checkResult = performSledCompatibilityCheck();

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_START_SLED_COMPATIBILITY_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    emit hideMessageBox();
    return true;
}

int StartUpDiagnostics::isOctPowerCheckedForStartUp() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_START_OCT_POWER_UP_CHECK);
}

int StartUpDiagnostics::isSledTurnedOn() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_START_OCT_SLED_ON_CHECK);
}

int StartUpDiagnostics::isSledVersionChecked() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_START_SLED_COMPATIBILITY_CHECK);
}
