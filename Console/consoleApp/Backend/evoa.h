/*
 * evoa.h
 *
 * Header for the EVOA controller system.
 *
 * Author: Ryan F. Radjabi
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#ifndef EVOA_H
#define EVOA_H

#include <QObject>

class EVOA : public QObject
{
    Q_OBJECT

public:
    EVOA();
    ~EVOA();

    void   calculateScaleFactor( int numChunks );
    double getCurrVoltage( void );
    void   setVoltageToDefault( void );
    float  getMinVal( void );
    float  getMaxVal( void );
    void   pauseEvoa( bool pause );

signals:
    void valueChanged( double val );
    void statusChanged( QString val );

public slots:
    void updateVoltage( double newVoltage );

private:
    bool   setVoltage(double val, QString statusLabel );
    double currVoltage_v;
    QString currStatus;
    double cachedEvoaVoltage_v;
    QString cachedEvoaStatus;
    double scale;
};

#endif // EVOA_H
