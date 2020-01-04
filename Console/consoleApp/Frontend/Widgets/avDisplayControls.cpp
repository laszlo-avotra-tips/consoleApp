/*
 * avDisplayControls.cpp
 *
 * Dual-handled slider to allow users to change the brightness and contrast
 * settings with one control since the brightness must always be less than
 * or equal to the contrast.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 *
 */
#include <QPalette>
#include "avDisplayControls.h"
#include "defaults.h"
#include "deviceSettings.h"


/*
 * constructor
 */
avDisplayControls::avDisplayControls( QWidget *parent ):
        QSlider(parent)
{
    this->setOrientation( Qt::Horizontal );

    // Set default range
    this->setRange( BrightnessLevels_HighSpeed.minValue, ContrastLevels_HighSpeed.maxValue );
//    this->setUpperValue( ContrastLevels_HighSpeed.defaultValue );
//    this->setLowerValue( BrightnessLevels_HighSpeed.defaultValue );

    // Set the color for the span when the slider is disabled
    QPalette qpalette;
    qpalette.setColor( QPalette::Disabled, QPalette::Highlight, SpanDisabledColor );
    qpalette.setColor( QPalette::Normal, QPalette::Highlight, SpanEnabledColor );
    this->setPalette( qpalette );
}

/*
 * updateBrightnessContrastLimits
 */
void avDisplayControls::updateBrightnessContrastLimits( void )
{
    deviceSettings &devSettings = deviceSettings::Instance();

    // Force the current values into the new range before resetting the range
    // Otherwise, if the new range is narrower it may cause side effects when applied
//    setLowerValue( 0 );
//    setUpperValue( 0 );

    if( devSettings.current()->isHighSpeed() )
    {
        setRange( BrightnessLevels_HighSpeed.minValue, ContrastLevels_HighSpeed.maxValue );
//        setUpperValue( ContrastLevels_HighSpeed.defaultValue );
//        setLowerValue( BrightnessLevels_HighSpeed.defaultValue );
    }
    else
    {
        setRange( BrightnessLevels_LowSpeed.minValue, ContrastLevels_LowSpeed.maxValue );
//        setUpperValue( ContrastLevels_LowSpeed.defaultValue );
//        setLowerValue( BrightnessLevels_LowSpeed.defaultValue );
    }

}
