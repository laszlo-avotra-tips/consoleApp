#ifndef ENDCASEDIAGNOSTICS_H
#define ENDCASEDIAGNOSTICS_H

#include <QObject>
#include "octsystemdiagnostics.h"

/*
* EndCaseDiagnostics class performs a pre-defined diagnostic checks at the end of a case
* We want to make sure OCT power and Sled are turned off right after processing a case
*/
class EndCaseDiagnostics : public OctSystemDiagnostics
{
    Q_OBJECT

public:
    /*
     * constructor to initalize base class OctSystemDiagnostics constructor.
     */
    explicit EndCaseDiagnostics(QObject *parent = nullptr);

    /*
     * Checks if OCT power down is verified as part of End case diagnostics.
     *
     * @return error code from OCT power down diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int isOctPowerCheckedForEndCase();

    /*
     * Check if Sled is turned off as part of End case diagnostics.
     *
     * @return error code from Sled turn off diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int isSledTurnedOff();

    /*
     * Performs all diagnostic checks as part of End case diagnostics.
     *
     * @return True for success; False otherwise.
     */
    bool performDiagnostics(bool checkAll) override;

private:

    /*
     * Internal helper to perform OCT power down check as part of End case diagnostics.
     *
     * @return error code from OCT power down diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int performOctPowerDownChecks();

    /*
     * Internal helper to turn off Sled as part of End case diagnostics.
     *
     * @return error code from Sled turning off diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int turnOffSled();
};

#endif // ENDCASEDIAGNOSTICS_H
