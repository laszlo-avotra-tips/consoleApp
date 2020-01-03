/*
 * keys.cpp
 *
 * Interface for reading, writing, checking cryptographic hash values
 * of files related to the software and cases.  The stored hash values
 * allow the software to determine if files or the stored values were
 * tampered with outside of the software's control. 
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include "keys.h"
#include "sawFile.h"
#include "logger.h"
#include <QCryptographicHash>
#include <QStringList>
//#include <QtConcurrentMap>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include "logger.h"
#include "util.h"

KeyBundle_t *checkSingleKey( KeyBundle_t *key );

namespace {
QMutex fileCheckMutex;
}

/*
 * Constructor
 */
Keys::Keys( QString sourceFilename, Access_T openMode ) :
        hFile( nullptr ),
        hFileInfo( nullptr ),
        fileCheckWatcher( nullptr ),
        isValidFiles( false ),
        isValueCalculated( false )
{
    keyFilename = sourceFilename;
    mode = openMode;
}

/*
 * Destructor
 */
Keys::~Keys( void )
{

    if (fileCheckWatcher ) {
        fileCheckWatcher->cancel();
    }

    // If the computation thread is running wait until it finishes before closing out
    while( isRunning() )
    {
        wait();
    }

    // Free up handles
    if( hFileInfo )
    {
        delete hFileInfo;
        hFileInfo = nullptr;
    }

    if( hFile )
    {
        delete hFile;
        hFile = nullptr;
    }
}

/*
 * init
 *
 * Initialize the object depending on the operating mode. The mode
 * is set in the constructor.
 */
void Keys::init( void )
{
    status = KeyOk;
    errorHandler &err = errorHandler::Instance();

    getFileHandles( keyFilename );

    if(  mode == ReadOnly )
    {
        if ( !hFile->open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            qDebug() << "ERROR: Could not open " << hFile->fileName() << " read-only";
            LOG( WARNING, QString( "ERROR: Could not open %1 read-only" ).arg( hFile->fileName() ) )
            err.warn(QString( tr( "Could not open %1 read-only." ) ).arg( hFile->fileName() ) );
            status = KeyFileError;
        }
        else
        {
            if( readKeys() )
            {
                // only validate the keys if the key file was valid
                isValidFiles = checkKeysBlocking();
            }
            else
            {
                LOG( WARNING, QString( "Key missing: %1 has no key data" ).arg( hFile->fileName() ) )
            }
        }
    }
    else if ( mode == WriteOnly )
    {
        // Create or Open the key file for this session
        if( !hFile->open( QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text ) )
        {
            qDebug() << "ERROR: Could not open " << hFile->fileName() << " for writing.";
            LOG( WARNING, QString( "ERROR: Could not open %1 for writing" ).arg( hFile->fileName() ) )
            err.warn( QString( tr( "Could not open %1 for writing." ) ).arg( hFile->fileName() ) );

            status = KeyFileError;
        }
        else
        {
            textStream.setDevice( hFile );
        }
    }
    else if ( mode == ReadWrite )
    {
        // Open the key file for appending
        if ( !hFile->open( QIODevice::ReadWrite | QIODevice::Text ) )
        {
            qDebug() << "ERROR: Could not open " << hFile->fileName() << " read/write: " << hFile->errorString();
            LOG( WARNING, QString( "ERROR: Could not open %1 read/write: %2" ).arg( hFile->fileName() ).arg( hFile->errorString() ) )
            err.warn( QString( tr( "Could not open %1 for read/write." ) ).arg( hFile->fileName() ) );
            status = KeyFileError;
        }
        else
        {
            if( !readKeys() )
            {
                LOG( WARNING, QString( "ERROR: Key file %1 has errors" ).arg( hFile->fileName() ) )
                err.warn( QString( "ERROR: Key file %1 has errors" ).arg( hFile->fileName() ) );
            }
        }
    }
    else
    {
        err.warn( tr( "Could not initialize the Key system with unknown mode." ) );
    }
}

/*
 * doKeyChecks
 */
void Keys::doKeyChecks( void )
{
    checkKeysBackground();
    textStream.setDevice( hFile );
}

/*
 * readKeys
 *
 * Read file names and cryptographic hashes from the key file into memory
 *
 * Return value: true  : keys found and loaded
 *               false : no keys found
 */
bool Keys::readKeys( void )
{
    bool keysFound = false;

    QTextStream data( hFile );
    QString strHashValue;
    QString strFilename;

    while( !data.atEnd() )
    {
        // Load the cryptographic hash string from the file
        data >> strHashValue;

        // The rest of the line is the filename; readLine() protects agains spaces in filenames
        strFilename = data.readLine();

        //qDebug() << "Loaded keys:" << strHashValue << strFilename.trimmed();

        // Protect against empty lines in the key file
        if( !strFilename.isEmpty() )
        {
            // Add this filename and cryptographic hash value to the internal hash
            keyHash.insert( strFilename.trimmed(), strHashValue.toLatin1() );
            keysFound = true;  // at least one key found
        }
    }

    return keysFound;
}

/*
 * checkSingleKey
 *
 * Callback for map/reduce to check a single file
 */
KeyBundle_t *checkSingleKey( KeyBundle_t *key )
{
    /*
     * Use the QtConcurrent Framework for backgrounding only.
     * Since this is an I/O operation, multiple instances
     * causes contention.
     */
    QMutexLocker lock(&fileCheckMutex);
    QByteArray testValue = SawFile::computeHash( key->file );

    if( key->key != testValue )
    {
        // TBD
        qDebug() << "ERROR: Key mismatch! " << key->file << "\n"
                << "    Loaded:" << key->key << "\n"
                << "  Computed:" << testValue;
        LOG( WARNING, QString( "ERROR: Key mismatch. %1" ).arg( key->file ) )
        key->valid = false;
    } else {
        qDebug() << "INFO: Key match " << key->file << "\n"
                << "    Loaded:" << key->key << "\n"
                << "  Computed:" << testValue;
        key->valid = true;
    }
    return key;
}

/*
 * handleFileCheckDone()
 *
 * Compute the result of the file check and notify the caller.
 */
void Keys::handleFileCheckDone( void )
{
    bool allValid = true;

    int itemCounter = keyHash.count();

    for ( int i = 0; i < itemCounter; i++) {
        if ( !( dynamic_cast<KeyBundle_t *>(fileCheckWatcher->resultAt( i )) )->valid ) {
            status = KeyCryptoError;
            allValid = false;
        }
    }
    emit fileCheckDone( allValid );

    delete fileCheckWatcher;
    fileCheckWatcher = nullptr;
    qDebug() << "File check complete, result is " << allValid;
    if ( allValid ) {
        LOG( INFO, QString( "File check complete, result is %1" ).arg( allValid ) )
    } else {
        LOG( WARNING, QString( "File check complete, result is %1" ).arg( allValid ) )
    }
}

/*
 * checkKeysBlocking
 *
 * Return value: true  : all stored cryptographic hash values matched the computed cryptographic hash values
 *               false : at least one value was incorrect
 *
 * This call blocks until all keys have been checked.
 */
bool Keys::checkKeysBlocking( void )
{
    bool retStatus = true;
    errorHandler & err = errorHandler::Instance();

    QHashIterator<QString, QByteArray> i( keyHash );

    while( i.hasNext() )
    {
        i.next();

        QString testFile = hFileInfo->absolutePath() + "/" + i.key();
        QByteArray testValue = SawFile::computeHash( testFile );

        if( i.value() != testValue )
        {
            err.warn( QString( tr( "Key mismatch: %1" ) ).arg( testFile ) );

            // Output key values to the debug system but not to the log files.
            // We can diagnose issues if we start up the MS Dbgview in the field.
            qDebug() << "ERROR: Key mismatch! " << testFile << "\n"
                    << "    Loaded:" << i.value() << "\n"
                    << "  Computed:" << testValue;

            retStatus = false;
        }
    }

    return retStatus;
}
/*
 * checkKeysBackground
 *
 * Check all keys in a background process. This function returns immediately,
 * status is conveyed asynchronously later. The caller is expected to link up
 * with this object to get status updates.
 */
void Keys::checkKeysBackground( void )
{
return; //lcv
//    QHashIterator<QString, QByteArray> i( keyHash );
//    QList<KeyBundle_t *> keyList;

//    while( i.hasNext() )
//    {
//        KeyBundle_t *keybundle = new KeyBundle_t;
//        i.next();
//        QString testFile = hFileInfo->absolutePath() + "/" + i.key();
//        keybundle->file = testFile;
//        QByteArray testValue = i.value();
//        keybundle->key = testValue;
//        keybundle->valid = false;
//        keyList.append(keybundle);
//    }

//    fileCheckWatcher = new QFutureWatcher<KeyBundle_t *>(this);
//    connect( fileCheckWatcher, SIGNAL( finished() ), this, SLOT( handleFileCheckDone() ) );
//    connect( fileCheckWatcher, SIGNAL( progressRangeChanged( int, int ) ), this, SIGNAL( fileCheckProgressRange( int,int) ) );
//    connect( fileCheckWatcher, SIGNAL( progressValueChanged( int ) ), this, SIGNAL( fileCheckProgressChanged( int ) ) );

//lcv    fileCheckWatcher->setFuture( QtConcurrent::mapped( keyList, checkSingleKey ) );
}

/*
 * addFile
 *
 * Add a file name to the internal hash
 */
void Keys::addFile( QString filename )
{
    // Set up the thread to compute the value and add it to the file
    FileItem_t fileToAdd;
    const QString AbsPath = hFileInfo->absolutePath() + "/";

    // The filename passed in should have a relative path; check that the absolute
    // is not included.  If it so, remove it.
    filename.remove( AbsPath, Qt::CaseInsensitive );
    fileToAdd.fileFullPath = AbsPath + filename;
    fileToAdd.fileFullPath = hFileInfo->absolutePath() + "/" + filename;
    fileToAdd.fileNameOnly = filename;

    mutex.lock();
    fileQ.enqueue( fileToAdd );
    mutex.unlock();

    if( !isRunning() )
    {
        // kick-off the thread
        start();
    }

}

/*
 * run
 *
 * Main process for the thread.  While the queue of files to add to the key list is
 * populated dequeue a name, compute its hash value, and store this information in
 * the key file.
 */
void Keys::run( void )
{
    while( !fileQ.empty() )
    {
        mutex.lock();
        FileItem_t f = fileQ.dequeue();
        mutex.unlock();

        QByteArray newValue  = SawFile::computeHash( f.fileFullPath );

        textStream << newValue << "  " << f.fileNameOnly << endl;
    }
}

/*
 * updateExistingKey
 *
 * Update the value of a key already present in the key file.
 */
void Keys::updateExistingKey( QString filename )
{

    textStream.seek(0);
    qDebug() << "Updating existing key for " << filename << ".";
    LOG( INFO, QString( "Updating existing key for %1" ).arg( filename ) )

    /*
     * Search the line containing the filename
     */
    while ( !textStream.atEnd() ) {
        QString line = textStream.readLine();
        if ( line.contains( filename ) ) {

            /*
             * Back up the file pointer to the beginning of the line
             */
            textStream.seek( textStream.pos() - ( line.length() + 2 ) );
            qDebug() << "Overwriting old key";
            break;
        }
    }

    if ( textStream.atEnd() ) {
        qDebug() << "Existing key entry for " << filename << " not found, will not update.";
        LOG( WARNING, QString( "Existing key entry for %1 not found, will not update" ).arg( filename ) )
        return;
    }

    /*
     * Write out the new key
     */
    QByteArray newKey = SawFile::computeHash( hFileInfo->absolutePath() + "/" + filename );
#ifdef Q_WS_MACX
    textStream << endl; // Required to account for DOS/UNIX end-of-line confusion
#endif
    textStream << newKey << "  " << filename << endl;
}

/*
 * computeKeyValues
 *
 * Compute and store the cryptographic values for each of the files listed
 * in the internal hash
 */
void Keys::computeKeyValues( void )
{
    QList<QString> keyList = keyHash.keys();

    // for each file name in the hash, compute the cryptographic value for
    // it and re-insert it back into the hash
    for( int i = 0; i < keyList.size(); i++ )
    {
        QString currFile     = keyList.at( i );
        QString currFullPath = hFileInfo->absolutePath() + "/" + currFile;
        QByteArray newValue  = SawFile::computeHash( currFullPath );

        keyHash.insert( currFile, newValue );
    }
    
    isValueCalculated = true;
}

/*
 * getFileHandles
 *
 * Establish handles to access information and data about and in a given file
 */
void Keys::getFileHandles( QString filename )
{
    errorHandler & err = errorHandler::Instance();

    // handle to get path/file information
    hFileInfo = new QFileInfo( filename );
    if( !hFileInfo )
    {
        qDebug() << "ERROR: Keys::hFileInfo (" << filename << ") is nullptr."; // TBD
        LOG( WARNING, QString( "ERROR: Keys::hFileInfo %1 is nullptr. " ).arg( filename ) )
        err.fail( QString( tr( "Keys::hFileInfo (%1) is nullptr. Key check failed." ) ).arg( filename ) );
        status = KeyFileError;
    }

    // handle to access the contents of the file
    hFile = new QFile( filename );
    if( !hFile )
    {
        LOG( WARNING, QString( "ERROR: Keys::hFile %1 is nullptr." ).arg( filename ) )
        err.fail( QString( tr( "Keys::hFile (%1) is nullptr. Key check failed." ) ).arg( filename) );
        status = KeyFileError;
    }
}

/*
* containsRequiredFiles
*
* Some key files have files that must exist in the list.  Once the key file
* has been loaded into the internal hash, this function can be called with
* a list of required files to ensure they are listed and have/will be checked.
*/
bool Keys::containsRequiredFiles( QStringList list )
{
    // default to OK
    bool retStatus = true;

    // Walk through the list of required files.  If any is not in the list,
    // log it and return false to the caller.
    for( int i = 0; i < list.count(); i++ )
    {
        retStatus = keyHash.contains( list.at( i ) );

        if( !status )
        {
            // Log with full path to the keys file
            LOG( WARNING, QString( "Missing required file %1 in %2 " ).arg( list.at( i ) ).arg( hFileInfo->absoluteFilePath() ) )
            break;
        }
    }

    return retStatus;
}
