/*
 * sawFile.cpp
 *
 * File and file system utilities.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#include <QCryptographicHash>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include "logger.h"
#include "sawFile.h"
#include "util.h"
#ifdef Q_WS_X11
#include <sys/statfs.h>
#include <errno.h>
#endif
#ifdef Q_WS_MAC
#include <sys/param.h>
#include <sys/mount.h>
#include <errno.h>
#endif

const QString NoncritialFileTypes[] = { "png", "mkv", "mp4" };

/*
 * computeHash
 *
 * Computes and returns the SHA1 hash of a file. This may
 * return 0 for non-critical files.
 */
QByteArray SawFile::computeHash( QString inputFile )
{
    errorHandler & err = errorHandler::Instance();
    QByteArray calcKey = 0;
    QFile *input       = new QFile( inputFile );
    QFileInfo filename( input->fileName() );

    bool isNoncriticalFile = false;

    /*
     * Check the filename against a list of non-critical file types.
     *
     */
    for( int kr = 0; kr < sizeof( NoncritialFileTypes ); kr++ )
    {
        isNoncriticalFile = filename.suffix().compare( NoncritialFileTypes[ kr ] );

        if( isNoncriticalFile )
        {
            break;
        }
    }

    if( input == NULL )
    {
        err.fail( QObject::tr( "OCTFile::computeHash() could not get input pointer." ) );
    }
    else if( !input->open( QIODevice::ReadOnly ) )
    {
        /*
         * Warn for noncritical files.  Fail for critical files.
         */
        if( isNoncriticalFile )
        {
            LOG( DEBUG, QString( "Non-critcal file could not open %1 for reading." ).arg( inputFile ) );
        }
        else
        {
            err.fail( QObject::tr( "Could not open %1 for reading." ).arg( inputFile ) );
        }
    }
    else
    {
        // Everything looks valid. Read the file and compute the hash
        const int BufferSize = 1024;
        char data[ BufferSize ];
        QCryptographicHash hash( QCryptographicHash::Sha1 );
        QDataStream in( input );

        int bytesRead = 0;

        // Read in the contents of the file and compute the hash
        while( !in.atEnd() )
        {
            // Request BufferSize bytes from the file, hash the amount we get back
            bytesRead = in.readRawData( data, BufferSize );
            hash.addData( data, bytesRead );
        }

        // Get the result in hex
        calcKey = hash.result().toHex();
    }

    // free the memory. NULL checked above
    delete input;

    return calcKey;
}

#ifdef WIN32
/*
 * getDiskFreeSpaceInGB
 *
 * Returns the amount of free drive space for the given drive in GB. The
 * value is rounded to the nearest integer value.
 */
int SawFile::getDiskFreeSpaceInGB( LPCWSTR drive )
{
    ULARGE_INTEGER freeBytes;
    freeBytes.QuadPart = 0L;

    int freeSpace_gb = 0;

    if( GetDiskFreeSpaceEx( drive, &freeBytes, NULL, NULL ) )
    {
        freeSpace_gb = freeBytes.QuadPart / B_per_GB;
    }
    qDebug() << "Free drive space: " << freeSpace_gb << "GB";

    return freeSpace_gb;
}
#else
int SawFile::getDiskFreeSpaceInGB( const char *path )
{
    struct statfs *stats = NULL;
    int freeSpace_gb     = 0;
    errorHandler & err = errorHandler::Instance();

    if( statfs( path, stats ) == 0 )
    {
        // success
        if( stats == NULL )
        {
            // fatal error
            err.fail( "OCTFile::getDiskFreeSpaceInGB(): statfs pointer is NULL." );
        }

        qDebug() << "Free drive space bavail: " << stats->f_bavail;
        freeSpace_gb = stats->f_bavail * (stats->f_bsize / 1024.0) / 1024.0 / 1024.0; //TBD: clean up!
     }
    else
     {
        // failure
        int errSave = errno;
        qDebug() << "statfs error:" << errSave;
     }


    qDebug() << "Free drive space: " << freeSpace_gb << "GB";

    return freeSpace_gb;
}
#endif
