/*
 * deviceSettings.cpp
 *
 * Stubs for unit tests that call the device settings object. Right now
 * the DSP test is the only consumer, so this doesn't do anything at all.
 * Other clients may have more deep usage, and the implementation that
 * is encapsulated in the header may need to move here (and the real .cpp)
 * to be filled out more.
 *
 * Author: Chris White
 *
 * Copyright 2010 Avinger
 */
#include "deviceSettings.h"

deviceSettings::deviceSettings(void) : QObject() {

}

deviceSettings::~deviceSettings() {

}

/*
 * setCustom
 *
 * Set the current device to a custom device, with the parameters
 * given.
 */
void deviceSettings::setCustom(int glueLineOffset, int position) {

}
