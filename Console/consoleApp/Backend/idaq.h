#ifndef IDAQ_H
#define IDAQ_H

#include <QThread>
#include <QString>
#include "buildflags.h"

class IDAQ : public QThread
{
    Q_OBJECT

public:
    virtual ~IDAQ(){}

    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void init() = 0;

    virtual QString getDaqLevel() = 0;

    virtual long getRecordLength() const = 0;

    virtual IDAQ* getSignalSource() { return nullptr;}

    virtual bool configure( void ) = 0;
    virtual bool getData( void ) = 0;

signals:
    void sendWarning( QString );
    void sendError( QString );
    void setBlackLevel( int );
    void setWhiteLevel( int );
    void setAveraging( bool );
    void setInvertColors( bool );
    void frameRate( int );
    void setFrameAverageWeights( int, int );
    void handleDisplayAngle( float );
    void signalDaqResetToFrontend( void );
    void updateCatheterView();
    void attenuateLaser( bool );

#if ENABLE_IPP_FFT_TUNING
    void magScaleValueChanged( int );
    void fftScaleValueChanged( int );
#endif

public slots:
    virtual void enableAuxTriggerAsTriggerEnable( bool ) = 0; //  * R&D only

};

#endif // IDAQ_H
