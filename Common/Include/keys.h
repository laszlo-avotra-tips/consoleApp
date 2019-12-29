/*
 * keys.h
 *
 * Read, write and compute cryptographic keys for files. These keys
 * are used to protect against tampering of system and user-generated
 * files. 
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2017 Avinger, Inc.
 *
 */
#ifndef KEYS_H_
#define KEYS_H_

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QThread>
#include <QQueue>
#include <QFuture>
#include <QFutureWatcher>

typedef struct
{
    QString file;
    QByteArray key;
    bool valid;
} KeyBundle_t;

class Keys : public QThread
{

Q_OBJECT

public:
    enum Access_T {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    enum Status {
        KeyOk,
        KeyFileError,
        KeyCryptoError
    };

    Keys( QString filename, Access_T openMode );
    ~Keys( void );
    void init( void );
    bool readKeys( void );
    bool checkKeysBlocking( void );
    void checkKeysBackground( void );
    bool isValid( void ) { return isValidFiles; }
    void doKeyChecks();
    bool containsRequiredFiles( QStringList list );
    void updateExistingKey( QString filename );
    enum Status getStatus( void ) {
        return status;
    }

public slots:
    void addFile( QString filename );

signals:
    void fileCheckDone( bool );
    void fileCheckProgressChanged( int );
    void fileCheckProgressRange( int, int );
    void sendWarning( QString );
    void sendError( QString );

protected:
    void run( void );

private slots:
    void handleFileCheckDone( void );

private:
    struct FileItem_t
    {
        QString fileFullPath;
        QString fileNameOnly;
    };

    enum Status status;

    QFile *hFile;
    QFileInfo *hFileInfo;
    QTextStream textStream;
    QHash< QString, QByteArray> keyHash;
    QFutureWatcher< KeyBundle_t *> *fileCheckWatcher;
    QString keyFilename;
    bool isValidFiles;
    bool isValueCalculated;
    Access_T mode;

    QQueue< FileItem_t > fileQ;

    void getFileHandles( QString filename );
    void computeKeyValues( void );

    // No default constructor
    Keys( void );

    QMutex mutex;
};

#endif // KEYS_H_
