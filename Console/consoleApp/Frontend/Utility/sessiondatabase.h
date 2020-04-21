/*
 * sessiondatabase.h
 *
 * Handles all database operations for image captures.  This is a singleton object
 * so it can be used throughout the codebase.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */

#ifndef SESSIONDATABASE_H
#define SESSIONDATABASE_H

#include <QtSql>

class sessionDatabase
{
public:

    sessionDatabase();
    ~sessionDatabase();

    QSqlError initDb(void);

    void createSession(void);
    void updateSession(void);
    void markExitAsClean(void);

    int addCapture(QString tag,
                    uint timestamp,
                    QString name,
                    QString deviceName,
                    bool isHighSpeed,
                    int pixelsPerMm );

    int addClipCapture(QString name,
                        uint timestamp,
                        QString catheterView,
                        QString deviceName,
                        bool isHighSpeed );

    void updateClipCapture( int lastClipID, int clipLength_ms );
    void updateCaptureTag( int key, QString newTag );
    void updateLoopTag( int key, QString newTag );

    int getNumCaptures(void);
    struct LoopStat
    {
        int numLoops;
        int totLength;
    };
    LoopStat getLoopsStats();
    void populateVersionTable( void );

private:

    // Hide copy and assign operator
    sessionDatabase(sessionDatabase const &);
    sessionDatabase& operator = (sessionDatabase const &);

    QString m_dbName;
};

#endif // SESSIONDATABASE_H
