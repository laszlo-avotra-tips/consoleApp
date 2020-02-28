/*
 * depthsetting.h
 *
 * Author: Ryan Radjabi
 *
 * Copyright (c) 2015-2018 Avinger, Inc.
 */
#ifndef DEPTHSETTING_H
#define DEPTHSETTING_H

#include <QObject>

class depthSetting : public QObject
{
    Q_OBJECT

public:
    // Singleton
    static depthSetting &Instance();

    float getImagingDepth_S( void ) const;
    int   getNumReticles( void ) const;
    int   getPixelsPerMm( void ) const;
    int   getCatheterEdgePosition( void ) const;
    float getFractionOfCanvas( void ) const;
    void  calculateReticles( void );
    float  getMinDepth_px( void ) const;
    float  getMaxDepth_px( void ) const;

public slots:
    void handleDeviceChange();
    void updateImagingDepth( double newDepth );

private:
    const float minDepth_px;
    const float maxDepth_px;
    float imagingDepth_S;
    const float fractionOfCanvas;

    // reticle vars
    int numReticles;
    int pixelsPerMm;
    int catheterEdgePosition;
    static depthSetting* theDepthManager;

    // prevent access to:
    depthSetting();  // default constructor
    ~depthSetting(); // default destructor
};


#endif // DEPTHSETTING_H
