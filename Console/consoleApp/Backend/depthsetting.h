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

    float getDepth_S( void )             { return imagingDepth_S; }
    int   getNumReticles( void )         { return numReticles; }
    int   getPixelsPerMm( void )         { return pixelsPerMm; }
    int   getCatheterEdgePosition( void ){ return catheterEdgePosition; }
    float getFractionOfCanvas( void )    { return fractionOfCanvas; }
    void  calculateReticles( void );
    float  getMinVal( void ) { return minDepth_px; }
    float  getMaxVal( void ) { return maxDepth_px; }

public slots:
    void handleDeviceChange();
    void updateImagingDepth( double newDepth );

private:
    float minDepth_px;
    float maxDepth_px;
    float imagingDepth_S;
    float fractionOfCanvas;

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
