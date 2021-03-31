#ifndef POWERUPDIAGNOSTICS_H
#define POWERUPDIAGNOSTICS_H

#include "octsystemdiagnostics.h"

/*
* PowerUpDiagnostics class performs a pre-defined diagnostic checks at the launch of OCT console app.
*/
class PowerUpDiagnostics : public OctSystemDiagnostics
{
    Q_OBJECT

public:
    /*
     * constructor to initalize base class OctSystemDiagnostics constructor.
     */
    explicit PowerUpDiagnostics(QObject *parent = nullptr);

    /*
     * Check if key checks are verified as part of power up diagnostics.
     *
     * @return error code from key check diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int areKeysVerified();

    /*
     * Check if interface board version is verified as part of power up diagnostics.
     *
     * @return error code from interface board diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int isInterfaceBoardChecked();

    /*
     * Check if system date is verified as part of power up diagnostics.
     *
     * @return error code from system date diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int isSystemDateChecked();

    /*
     * Performs all diagnostic checks as part of power up diagnostic checks.
     *
     * @return True for success; False otherwise.
     */
    bool performDiagnostics(bool checkAll) override;

private:

    /*
     * Internal helper to perform key checks as part of power up diagnostics.
     *
     * @return error code from key check diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int verifyAssets();

    /*
     * Internal helper to perform interface board compatibility check as part of power up diagnostics.
     *
     * @return error code from interface board diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int performInterfaceBoardCompatibilityCheck();

    /*
     * Internal helper to perform system date check as part of power up diagnostics.
     *
     * @return error code from interface board diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int performSystemDateCheck();
};

#endif // POWERUPDIAGNOSTICS_H
