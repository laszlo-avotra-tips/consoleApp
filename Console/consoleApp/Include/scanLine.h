
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

/*
 *
 */
class scanframe
{
public:
    scanframe() = default;
    ~scanframe()
    {
        if( m_dispData )
        {
            delete m_dispData;
            m_dispData = nullptr;
        }

        if( m_videoData )
        {
            delete m_videoData;
            m_videoData = nullptr;
        }

    }

    unsigned short angle{0};
    bool           isPlayback{false};
    unsigned int   depth{0};
    unsigned int   width{0}; // in angular elements.
    unsigned long  timestamp{0};
    unsigned long  frameCount{0};
    QByteArray     *m_dispData{nullptr};
    QByteArray     *m_videoData{nullptr};
};

#endif // SCANLINE_H_
