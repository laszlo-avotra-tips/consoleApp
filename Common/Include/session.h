/*
 * session.h
 *
 *
 * Author(s): Dennis W. Jackson
 *
 * Copyright (c) 2010-2017 Avinger, Inc.
 *
 */
#ifndef SESSION_H_
#define SESSION_H_

#include <QObject>
#include <QMutex>
#include "keys.h"
#include "eventDataLog.h"

class Session : public QObject
{
    Q_OBJECT

public:
    Session();
    ~Session();
    void init( void );
    void start();
    void shutdown( void );

    EventDataLog *getCurrentEventLog( void ) { return eventLog; }

public slots:
    void handleFileToKey(QString filename);

signals:
    void sendSessionEvent( QString );
    void sendWarning( QString );
    void sendError( QString );

private:
    EventDataLog *eventLog;
    Keys *keys;
    QMutex mutex;

    // There is only one session
    Session( Session const & ); // hide copy
    Session & operator=( Session const & ); // hide assign

};

#endif  // SESSION_H_
