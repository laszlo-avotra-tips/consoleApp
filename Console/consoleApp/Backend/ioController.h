/*
 * ioController.h
 *
 * Interface for the Data Translations USB GPIO module that controls the EVOA and Laser Power.
 *
 * Author: Ryan F. Radjabi
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#ifndef IOCONTROLLER_H
#define IOCONTROLLER_H

#include <QObject>
#include <QMutex>
#include <Windows.h>
//#include "OLDAAPI.H"

class ioController : public QObject
{
    Q_OBJECT

public:
    // Singleton
    static ioController &Instance();

    enum ControllerMode
    {
        AnalogOutput,
        DigitalOutput
    };

    struct ioModule
    {
        ControllerMode mode;
//        olss_tag       tag;
        bool           isEnabled;
    };

    ioModule analogOutModule;
    ioModule digitalOutModule;

    bool queryDevice( void );
    bool configureModule( ioModule *module );
    bool shutdown( void );
    bool isReady( void );
    bool setAnalogVoltageOut(  double val );
    bool setDigitalVoltageOut( long   val );    

signals:
    void sendError( QString );

private:
    QMutex mutex;
    LPSTR  ioControllerLpszName;
    LPSTR  ioControllerLpszEntry;
    LPARAM ioControllerLParam;

    bool     deviceIsConfigured;
    static ioController* theController;

    void disableAllModules( void );

                    // prevent access to:
    ioController();     //   default constructor
    ~ioController();    //   default destructor
};

#endif // IOCONTROLLER_H
