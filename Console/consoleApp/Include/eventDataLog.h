/*
 * eventDataLog.h
 *
 * Author(s): Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */

#ifndef EVENTDATALOG_H_
#define EVENTDATALOG_H_

#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamWriter>

class EventDataLog
{
public:
    EventDataLog();
    ~EventDataLog();
    void init( void );
    void addEvent( QString eventName,
                   unsigned long frameCount,
                   unsigned long timeStamp,
                   QString dataFileName );
    void close( void );

private:
    QFile *hFile;
    QXmlStreamWriter *xmlWriter;

    EventDataLog( EventDataLog const & ); // hide copy
    EventDataLog & operator=( EventDataLog const & ); // hide assign
};

#endif // EVENTDATALOG_H_
