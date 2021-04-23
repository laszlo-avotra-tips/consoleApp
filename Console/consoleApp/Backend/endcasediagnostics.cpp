#include "endcasediagnostics.h"
#include "logger.h"

EndCaseDiagnostics::EndCaseDiagnostics(QObject *parent) : OctSystemDiagnostics(parent) {
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_END_CASE_OCT_POWER_DOWN_CHECK, "OCT_END_CASE_OCT_POWER_DOWN_CHECK");
    addDiagnosticCheckType(OctDiagnosticCheckType::OCT_START_OCT_SLED_OFF_CHECK, "OCT_START_OCT_SLED_OFF_CHECK");
    diagnosticModule = "End case diagnostic check";
}

int EndCaseDiagnostics::performOctPowerDownChecks() {
    auto interfaceSupport = InterfaceSupport::getInstance();

    if (interfaceSupport) {
        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_END_CASE_OCT_POWER_DOWN_CHECK;
        bool operationResult = interfaceSupport->turnOnACPowerToOCT(false);

        if (operationResult) {
            LOG(INFO, "Successfully turned off AC power to OCT");
            operationResult = interfaceSupport->setVOAMode(false);

            if (operationResult) {
                LOG( INFO, "Successfully set VOA to blocking mode");
            } else {
                // InterfaceSupport cannot turn on AC power to OCT.
                // Flag this as error
                statusMessage = tr( "Failed to set VOA to blocking mode");
                return 0;
            }
        } else {
            // InterfaceSupport cannot turn on AC power to OCT.
            // Flag this as error
            statusMessage = tr( "Failed to turn off AC power to OCT");
            return 0;
        }
    } else {
        // InterfaceSupport cannot be instatiated possibly due to FTDI error.
        // Flag this as test not run
        return -1;
    }
    return 1; // We passed all checks
}

int EndCaseDiagnostics::turnOffSled() {
    auto interfaceSupport = InterfaceSupport::getInstance();

    if (interfaceSupport) {
        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_START_OCT_SLED_OFF_CHECK;
        bool operationResult = interfaceSupport->turnOnOffSled(false);

        if (operationResult) {
            LOG( INFO, "Successfully turned off Sled");
        } else {
            LOG( ERROR, "Failed to turn off Sled");
            statusMessage = tr( "Failed to turn off Sled");
            return 0;
        }

        operationResult = interfaceSupport->turnOnOffLaser(false);

        if (operationResult) {
            LOG( INFO, "Successfully turned off Laser");
        } else {
            LOG( ERROR, "Failed to turn off Laser");
            statusMessage = tr( "Failed to turn off Laser");
            return 0;
        }
    } else {
        // InterfaceSupport cannot be instatiated possibly due to FTDI error.
        // Flag this as test not run
        return -1;
    }

    return 1; // We passed the check
}

bool EndCaseDiagnostics::performDiagnostics(bool checkAll) {
    if (checkAll) {
        resetResultsTable();
    }

    bool checkResult = performOctPowerDownChecks();

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_END_CASE_OCT_POWER_DOWN_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    checkResult = turnOffSled();

    if (!setInitCheckStatusAndProcessResult(OctDiagnosticCheckType::OCT_START_OCT_SLED_OFF_CHECK,
                                            "Shutdown", checkResult)) {
        return false;
    }

    emit hideMessageBox();
    return true;
}

int EndCaseDiagnostics::isOctPowerCheckedForEndCase() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_END_CASE_OCT_POWER_DOWN_CHECK);
}

int EndCaseDiagnostics::isSledTurnedOff() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_START_OCT_SLED_OFF_CHECK);
}
