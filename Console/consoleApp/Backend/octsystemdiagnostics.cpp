#include "octsystemdiagnostics.h"
#include "logger.h"

OctSystemDiagnostics::OctSystemDiagnostics(QObject *parent) : QObject(parent) {
}

void OctSystemDiagnostics::addDiagnosticCheckType(OctDiagnosticCheckType diagnosticCheckType,
                                                  QString diagnosticCheckDesc, int numRetries) {
    if (octDiagnosticCheckRetries.find(diagnosticCheckType) == octDiagnosticCheckRetries.end()) {
        octDiagnosticChecksResultTable.emplace_back(std::pair<OctDiagnosticCheckType, int> (diagnosticCheckType, -1));
        octDiagnosticCheckRetries.emplace(diagnosticCheckType, std::pair<QString, int>(diagnosticCheckDesc, numRetries));
    }
}

void OctSystemDiagnostics::onUserAcknowledged() {
    QString currentPowerUpCheck = getOctDiagnosticCheckInfo();

    currentRetriesAttempted++;

    if (currentOctDiagnosticCheck == OctDiagnosticCheckType::OCT_INIT_SYSTEM_DATE_CHECK) {
        return;
    }

    performDiagnostics(false);

    if (currentRetriesAttempted == octDiagnosticCheckRetries[currentOctDiagnosticCheck].second) {
        // Now send a message back to the message interface to shutdown
        LOG3(currentPowerUpCheck, "Exceeded all retries", currentRetriesAttempted);
        emit showMessageBox(statusMessage, diagnosticModule, "Shutdown");
    } else {
        LOG2(currentPowerUpCheck, currentRetriesAttempted);
    }
}

int OctSystemDiagnostics::getOctDiagnosticCheckStatus(OctDiagnosticCheckType initCheckType) {
    for (const auto& item : octDiagnosticChecksResultTable) {
        if (item.first == initCheckType) {
            return item.second;
        }
    }

    return -1;
}

/*
 * performACPowerCheck
 *
 * Performs AC power check; If application is running without AC power, then it should flag
 * it as a critical error. Application must terminate once this condition is detected
 */
int OctSystemDiagnostics::performACPowerCheck() {
    int result = getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_INIT_AC_POWER_CHECK);

    if (result == -1) {
        currentRetriesAttempted = 0;
    }

    if (result != 1) {
        float minimumACVoltage = (float) 122 * 0.9f;

        auto interfaceSupport = InterfaceSupport::getInstance();
        float detectedACVoltage = 0.0;

        if (interfaceSupport) {
            detectedACVoltage = interfaceSupport->getSupplyVoltage();
        } else {
            // InterfaceSupport cannot be instatiated possibly due to FTDI error.
            // Flag this as test not run
            return -1;
        }
        currentOctDiagnosticCheck = OctDiagnosticCheckType::OCT_INIT_AC_POWER_CHECK;

        if (minimumACVoltage > detectedACVoltage) {
            LOG2(minimumACVoltage, detectedACVoltage);
            statusMessage = tr( "Interface board AC power check failed" );
            LOG(WARNING, "Interface board AC power check failed");
            return 0; // Failure
        }

        return 1; // Success
    }

    return result;
}

bool OctSystemDiagnostics::setInitCheckStatusAndProcessResult(const OctDiagnosticCheckType& initCheckType,
                                                            const QString& actionString, int result) {
    for (auto &item : octDiagnosticChecksResultTable) {
        if (item.first == initCheckType) {
            item.second = result;
            break;
        }
    }

    if(result == 0) {
        if (octDiagnosticCheckRetries[initCheckType].second == 1) {
            emit showMessageBox(statusMessage, diagnosticModule, actionString);
        } else {
            emit showMessageBox(statusMessage, diagnosticModule, "Warning");
        }
        return false;
    }
    return true;
}

void OctSystemDiagnostics::resetResultsTable() {
    for (auto& item : octDiagnosticChecksResultTable) {
        item.second = -1;
    }
}

const QString OctSystemDiagnostics::getOctDiagnosticCheckInfo() {
    QString type = "";

    if (octDiagnosticCheckRetries.find(currentOctDiagnosticCheck) != octDiagnosticCheckRetries.end()) {
        type = octDiagnosticCheckRetries[currentOctDiagnosticCheck].first;
    }

    return type;
}

int OctSystemDiagnostics::isACPowerChecked() {
    return getOctDiagnosticCheckStatus(OctDiagnosticCheckType::OCT_INIT_AC_POWER_CHECK);
}
