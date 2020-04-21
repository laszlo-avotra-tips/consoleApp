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

    // singleton
//    static sessionDatabase & Instance(void);
    // do not hide ctor
    sessionDatabase();
    ~sessionDatabase();

    QSqlError initDb(void);
    void close( void );

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

private:
    // hide ctor
//    sessionDatabase();

    void populateVersionTable( void );

    // Hide copy and assign operator
    sessionDatabase(sessionDatabase const &);
    sessionDatabase& operator = (sessionDatabase const &);

    // Searchable database
    QSqlDatabase db;

//    sessionDatabase* theDB{nullptr};
};

#endif // SESSIONDATABASE_H
