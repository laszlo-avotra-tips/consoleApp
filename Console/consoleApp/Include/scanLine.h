
/*
 * scanline.h
 *
 * Implements an object consisting of a single OCT A-line's worth of data,
 * together with any meta-data (angle, timing, etc.) that may be associated 
 * with it.
 *
 * Also includes a full frame data object, consisting of one rotations
 * worth of scan lines.
 *
 * Author: Chris White
 * Copyright (c) 2011-2018 Avinger Inc.
 */
#ifndef SCANLINE_H_
#define SCANLINE_H_

#include <QByteArray>
#include "buildflags.h"

/*
 *
 */
class scanframe
{
public:
    scanframe()
    {
        dispData = NULL;
    }
    ~scanframe()
    {
        if( dispData )
        {
            delete dispData;
            dispData = NULL;
        }

        if( videoData )
        {
            delete videoData;
            videoData = NULL;
        }

    }

    unsigned short angle;
    bool           isPlayback;
    unsigned int   depth;
    unsigned int   width; // in angular elements.
    unsigned long  timestamp;
    unsigned long  frameCount;
    QByteArray     *dispData;
    QByteArray     *videoData;
};

#endif // SCANLINE_H_
