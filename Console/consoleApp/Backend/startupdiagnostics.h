#ifndef STARTUPDIAGNOSTICS_H
#define STARTUPDIAGNOSTICS_H

#include <QObject>

#include "octsystemdiagnostics.h"

/*
* StartUpDiagnostics class performs a pre-defined diagnostic checks at the start of device configuration.
* before a case is run from OCT
*/
class StartUpDiagnostics : public OctSystemDiagnostics
{
    Q_OBJECT

public:
    /*
     * constructor to initalize base class OctSystemDiagnostics constructor.
     */
    explicit StartUpDiagnostics(QObject *parent = nullptr);

    /*
     * Check if OCT power is verified as part of start up diagnostics.
     *
     * @return error code from OCT power up diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int isOctPowerCheckedForStartUp();

    /*
     * Check if Sled can be turned on as part of start up diagnostics.
     *
     * @return error code from Sled turning on diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int isSledTurnedOn();

    /*
     * Check if Sled version is verified as part of start up diagnostics.
     *
     * @return error code from Sled compatibility diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int isSledVersionChecked();

    /*
     * Performs all diagnostic checks as part of start up diagnostics.
     *
     * @return True for success; False otherwise.
     */
    bool performDiagnostics(bool checkAll) override;

private:

    /*
     * Internal helper to perform OCT power up check as part of start up diagnostics.
     *
     * @return error code from OCT power up diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int performOctPowerUpChecks();

    /*
     * Internal helper to turn on Sled as part of start up diagnostics.
     *
     * @return error code from Sled turning on diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int turnOnSled();

    /*
     * Internal helper to perform Sled compatibility check as part of start up diagnostics.
     *
     * @return error code from Sled compatibility diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int performSledCompatibilityCheck();
};

#endif // STARTUPDIAGNOSTICS_H
