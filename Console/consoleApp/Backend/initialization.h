/*
 * initialization.h
 *
 * Runs required checks before the main application starts
 * to make sure the system is ready to run.  It also has a thread
 * that runs additional set-up in the background; items that fall
 * into this category must not be needed for the Case Info or
 * initial device set-up.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */

#include <QString>
#include <QThread>
#include "trigLookupTable.h"

#ifndef INITIALIZATION_H_
#define INITIALIZATION_H_

class Initialization : public QThread
{
    Q_OBJECT

public:
    Initialization();
    bool init();
    bool warningPosted( void ) { return hasWarning; }
    QString getStatusMessage( void ) { return statusMessage; }
    bool isPhysicianScreenAvailable( void ) { return docScreenAvailable; }

signals:

private:
    bool hasWarning;
    bool docScreenAvailable;
    QString statusMessage;

protected:
    void run() 
    { 
        // compute the sector look-up tables before they are needed
        trigLookupTable::Instance(); 
    }

};

#endif //INITIALIZATION_H_
