#ifndef OCTSYSTEMDIAGNOSTICS_H
#define OCTSYSTEMDIAGNOSTICS_H

#include <QObject>
#include <map>
#include <vector>

#include "interfacesupport.h"

class QString;

/*
* Enum declarations for various diagnostic checks to be performed during power up
* start up and end case stages of OCT console application
*/
enum class OctDiagnosticCheckType {
    // Protyping correct return codes
    OCT_INIT_CHECK_UNKNOWN = 0,
    OCT_INIT_KEY_CHECK = 1,
    OCT_INIT_IF_BOARD_COMPATIBILITY_CHECK = 2,
    OCT_INIT_AC_POWER_CHECK = 3, // 5 times check failure
    OCT_INIT_SYSTEM_DATE_CHECK = 4,
    OCT_START_OCT_POWER_UP_CHECK = 5,
    OCT_START_OCT_SLED_ON_CHECK = 6,
    OCT_START_SLED_COMPATIBILITY_CHECK = 7,
    OCT_END_CASE_OCT_POWER_DOWN_CHECK = 8,
    OCT_START_OCT_SLED_OFF_CHECK = 9
};

using OctDiagnosticCheckRetryList = std::map<OctDiagnosticCheckType, std::pair<QString, int>>;
using OctDiagnosticChecksResultTable = std::vector<std::pair<OctDiagnosticCheckType, int>>;

/*
* OctSystemDiagnostics class serves as base class to perform various diagnostic checks for OCT console app.
* Derived instance of this class is hooked up to a custom message box that reports failures of these diagnostic checks
*/
class OctSystemDiagnostics : public QObject {
    Q_OBJECT

public:

    /*
     * Each derived class must implement this virtual function to perform a pre-defined set of diagnostic checks.
     *
     * @param checkAll
     *      Boolean flag to indicate whether or not tp perform complete checks or to continue from last failed check.
     *
     * @return True if all diagnostic checks succeed
     */
    virtual bool performDiagnostics(bool checkAll) = 0;


public slots:
    /*
     * Signal function from the messaging layer to the diagnostic module
     */
    void onUserAcknowledged();

signals:

    /*
     * Signal function to the messaging layer to report any failure of a diagnostic check.
     *
     * @param msg
     *      Message to be displayed by the messaging layer.
     *
     * @param module
     *      Diagnostic module that triggered this signal. This is for information purpose
     *
     * @param actionString
     *      GUI action that messaging layer will trigger. Action string could be either "Shutdown" or "Warning"
     *
     */
    void showMessageBox(const QString& msg, const QString& module, const QString& actionString);

    /*
     * Signal function to the messaging layer to hide any error message upon a successful diagnostic check.
     */
    void hideMessageBox();

protected:
    /*
     * Protected constructor to be used by derived classes.
     */
    OctSystemDiagnostics(QObject *parent = nullptr);

    /*
     * Each derived must call this base class function to add a particular diagnostic check.
     *
     * @param diagnosticCheckType
     *      Type of diagnostic check to be performed by the derived class.
     *
     * @param diagnosticCheckDesc
     *      Description text for the diagnostic check.
     *
     * @param numRetries
     *      Number of retries along with a warning message issued for this diagnostic check.
     *      Once we get past retry counts, diagnostic check should send a shutdown message
     *      to the messaging interface to prompt the user about shutting down OCT console app
     *
     */
    void addDiagnosticCheckType(OctDiagnosticCheckType diagnosticCheckType,
                                QString diagnosticCheckDesc, int numRetries = 1);

    /*
     * Check the status of AC power check performed by Start up and power up diagnostic check.
     *
     * @return error code from this diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int isACPowerChecked();

    /*
     * Check the status of any diagnostic check performed by Power up/Start up/End case diagnostic checks.
     *
     * @param initCheckType
     *      Type of diagnostic check for which status to be queried.
     *
     * @return error code from this diagnostic check.
     * Return "1" for success, Return "0" for failure, "-1" test not run
     *
     */
    int getOctDiagnosticCheckStatus(OctDiagnosticCheckType initCheckType);

    /*
     * Helper debug function to return string information for current diagnostic check.
     *
     * @return string representation of current diagnostic check.
     */
    const QString getOctDiagnosticCheckInfo();

    /*
     * Internal helper function called by derived class to check the status and send error
     * message to the messaging layer if a particular diagnostic check had failed.
     *
     * @param initCheckType
     *      Type of diagnostic check for which status to be queried.
     *
     * @param actionString
     *      Type of action Warning/Shutdown sent to messaging layer.
     *
     * @param result
     *      Error code returned from a diagnostic check.
     *
     * @return True if diagnostic check had succeeded; False otherwise.
     */
    bool setInitCheckStatusAndProcessResult(const OctDiagnosticCheckType& initCheckType,
                                            const QString& actionString, int result);

    /*
     * Perform AC power check. This is perfmored by both Power up and Start up diagnostic checks
     *
     * @return error code from this diagnostic check.
     *      Return "1" for success, Return "0" for failure, "-1" test not run
     */
    int performACPowerCheck();

    /*
     * Internal helper function to reset results table
     */
    void resetResultsTable();

    QString statusMessage = ""; // Message to be sent to sent to the message layer for failure reporting
    QString diagnosticModule = ""; // Diagnostic module name of derived class
    OctDiagnosticCheckRetryList octDiagnosticCheckRetries; // List tracking retry counts for each diagnostic check
    int currentRetriesAttempted = 0; // Current retry count attempted
    OctDiagnosticCheckType currentOctDiagnosticCheck; // Current diagnostic check being performed
    OctDiagnosticChecksResultTable octDiagnosticChecksResultTable; // Results table for all diagnostic checks
};

#endif // OCTSYSTEMDIAGNOSTICS_H
