/*
 * depthsetting.cpp
 *
 * Author: Ryan Radjabi
 *
 * Copyright (c) 2015-2018 Avinger, Inc.
 */
#include "depthsetting.h"
#include "deviceSettings.h"
#include "defaults.h"

/*
 * Constructor
 */
depthSetting::depthSetting()
{
    /*
     * Number of pixels to display beyond the imaging mask.
     *
     * Min Depth is the minimum depth desired to image. If we want about 1 mm, use about 156px.
     *
     * Max Depth is the maximum depth desired to image. This should theoretically be (at most) 1024 - internal mask. We should
     * never display 1024 pixels, or we would be drawing pixels inside the mask.
     *
     * Note minDepth_px and maxDepth_px are unused for Low Speed devices.
     */
    minDepth_px = 300; // 300 is a little less than 2 mm
    maxDepth_px = 600; // 600 is ~3.75mm

    // 0.475 out of 0.50 is used in Warp.CL, that extra 0.025 is reserved for cardinal tick marks.
    fractionOfCanvas = 0.475f;

    imagingDepth_S = minDepth_px; // set the current depth
    calculateReticles();
}

/*
 * Destructor
 */
depthSetting::~depthSetting()
{
    // empty
}

/*
 * updateImagingDepth
 *
 * This is a slot that is connected to the controlling widget.
 */
void depthSetting::updateImagingDepth( double newDepth )
{
    imagingDepth_S = (float)newDepth;
    calculateReticles();
}

/*
 * handleDeviceChange
 */
void depthSetting::handleDeviceChange()
{
    imagingDepth_S = 450;  // Set the initial value half-way between the min and max range.
    calculateReticles();
}

/*
 * calculateReticles
 *
 * Run once at device select. Compute catheterEdgePosition, numReticles,
 * and pixelsPerMm for use in paintSector().
 *
 * Called at device select/change and when imaging depth is updated.
 */
void depthSetting::calculateReticles( void )
{
    deviceSettings &dev = deviceSettings::Instance();

    // Make sure a device exists before calling member functions.
    if( dev.current() != NULL )
    {
        if( dev.current()->isHighSpeed() )
        {
            /* XXX MAGIC:
             *  - 0.5 is 1/2
             *  - 1000.0 is umPerMm
             */
            const float percentageOfCanvasUsed = ( fractionOfCanvas / 0.5f ); // because we reserve space for the cardinal marks, we don't use entire canvas.
            const float StandardMmPerSample = (float)dev.current()->getImagingDepthNormal_mm() / (float)dev.current()->getALineLengthNormal_px();
            const float imagingDepthMm = (float)imagingDepth_S * (float)StandardMmPerSample;
            const float catheterRadius_mm = (float)dev.current()->getCatheterRadius_um() / 1000.0f;
            const float distanceFromCenterInMm = (float)catheterRadius_mm + (float)( (float)imagingDepth_S * StandardMmPerSample );
            const float distanceFromCenterInPx = (int)( ( (float)SectorWidth_px / 2.0f ) * percentageOfCanvasUsed );
            const float pxPerMm = distanceFromCenterInPx / distanceFromCenterInMm;

            numReticles = (int)imagingDepthMm;
            pixelsPerMm = (int)pxPerMm;
            catheterEdgePosition = (int)( catheterRadius_mm * pxPerMm );
        }
        else
        {
            // NOTE Drawing 1 to 1, so 1 px = 1 S
            const float StandardMmPerSample = (float)dev.current()->getImagingDepthNormal_mm() / (float)dev.current()->getALineLengthNormal_px();

            numReticles = (int)( dev.current()->getImagingDepthNormal_mm() + 0.5 );
            pixelsPerMm = (int)( 1.0f / StandardMmPerSample );
            catheterEdgePosition = dev.current()->getCatheterRadius_px();
        }
    }
}
