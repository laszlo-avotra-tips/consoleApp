/*
 * octFile.cpp
 *
 * Description:
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger Inc.
 *
 */
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include "Utility/userSettings.h"
#include "defaults.h"
#include "logger.h"
#include "octFile.h"
#include "util.h"
#include "styledmessagebox.h"

// ID for the any programs that parses the data
const unsigned char OctFileFormatVersion = 4;

// The maximum file size is arbitrary; however, some factors that went into
// setting it include:
//   - the larger the file, the longer the key hashing process takes to run.
//   - it is advantageous to be able to load a single file in a hex editor for
//     development purposes
//   - FAT32 has an individual file size limit of 2 GB

//lcv const unsigned long MaxFileSize_MB = 128;
//lcv const unsigned long MaxFileSize_B  = MaxFileSize_MB * B_per_KB * KB_per_MB;

/*
 * Constructor -- Console mode, creating data
 */
OCTFile::OCTFile( unsigned short numLinesPerRev )
{
    // Populate the header structure for all OCT files created in this session
    memcpy( header.magic, "STL", sizeof( header.magic ) );
    header.formatVersion         = OctFileFormatVersion;
    header.numLinesPerRevolution = numLinesPerRev;

    storageDir       = "";
    namePrefix       = "";
    fileId           = "";
    fileIteration    = 0;
}

/*
 * Destructor
 */
OCTFile::~OCTFile( void )
{
    close();
    storageDir = "";
}

/*
 * open
 *
 * Create and open a file for writing. If successful, the OCT file header is
 * written to the file.
 */
void OCTFile::open( void )
{
    errorHandler & err = errorHandler::Instance();

    // If a storage location has not been set, use the case directory
    if( storageDir == "" )
    {
        // set the default directory to the case directory
        caseInfo &info = caseInfo::Instance();
        storageDir = info.getStorageDir();
    }

    // restart the frame counter for this file
    header.numFramesWritten = 0;

    currFullyQualifiedFileName = genFullyQualifiedFileName();

    hFile = new QFile( currFullyQualifiedFileName );
    if( !hFile )
    {
        // fatal error
        err.fail( tr( "Could not create a new file handle for saving OCT data." ), true );
    }
    if( !hFile->open( QIODevice::WriteOnly ) )
    {
        displayFailureMessage( tr( "Could not open OCT file for writing." ), true );
    }
    outputStream.setDevice( hFile );
    outputStream.setVersion( QDataStream::Qt_4_6 );
    writeFileHeader();
}

/*
 * genFullyQualifiedFileName
 *
 * Create a unique filename for storing data.  The string that is returned
 * contains the full path to the file as well as the name.  Names are made
 * unique by adding a three digit numerical suffix; the current suffix count
 * is maintained by the OCTFile object.  This routine creates unique names
 * from counts 000 to 999.
 */
QString OCTFile::genFullyQualifiedFileName( void )
{
    // convert the current file iteration value to a zero-padded string
    QString strIter = QString( "%1" ).arg( fileIteration, 3, 10, QLatin1Char( '0' ) );

    currFileName = namePrefix + fileId + strIter + ".oct";

    // return the full path and file name to use
    return( storageDir + "/" + currFileName );
}

/*
 * writeFileHeader
 *
 * Write the file header to the file.  The number of frames field is left 
 * empty; this is filled when the file is closed.
 */
void OCTFile::writeFileHeader( void )
{
    outputStream.writeRawData( header.magic, 3 );
    outputStream << header.formatVersion
                 << header.numLinesPerRevolution
                 << quint32(0);  // Number of frames in this file; this
                                 // will be filled in when the file is closed
}

/*
 * writeNumFramesWritten
 *
 * Update the file header with the number of frames recorded to the file. This
 * is called when the file is closed.
 */
void OCTFile::writeNumFramesWritten( void )
{
    unsigned int offset = sizeof( header.magic ) + 
                          sizeof( header.formatVersion ) + 
                          sizeof( header.numLinesPerRevolution );

    hFile->seek( offset );
    outputStream << header.numFramesWritten;
}


/*
 * close
 *
 * Update the file header, flush the contents of the file and close it out.
 * Remove any associated file handles as well.
 */
void OCTFile::close( void )
{
    writeNumFramesWritten();
    hFile->flush();
    hFile->close();

    // send full path for file
    emit sendFileToKey( storageDir + "/" + currFileName );

    if( hFile )
    {
        delete hFile;
        hFile = nullptr;
    }
}


/*
 * processHeaders
 *
 * Process all file headers in the set to get
 * aggregate counts, etc.
 */
void OCTFile::processHeaders()
{
    char magic[] = { '\0', '\0', '\0', '\0' };
    unsigned char currentVersion = 0;
    unsigned char version = 0;
    unsigned short tmpLinesPerRev;

    QFile *filePtr;

    foreach( filePtr, hFileList )
    {
        int numFramesThisFile = 0;
        QDataStream in( filePtr );

        in.setVersion( QDataStream::Qt_4_6 );
        in.readRawData( magic, 3 );
        in >> version
           >> tmpLinesPerRev
           >> numFramesThisFile;

        if (currentVersion == 0) {
            currentVersion = version;
        }

        totalNumberOfFrames += quint32(numFramesThisFile);

        indexList.append( int(totalNumberOfFrames ) );
    }
}

/*
 * resetFile
 *
 * Seeks to the first frame in the file
 */
void OCTFile::resetFile()
{
    unsigned char dummychar;
    quint32 dummyint;
    quint16 dummyshort;
    char magic[] = { '\0', '\0', '\0', '\0' };

    hFile->seek( 0 );

    // Snarf the header again
    QDataStream in( hFile );
    in.readRawData( magic, 3 );
    in >> dummychar
       >> dummyshort
       >> dummyint;
}

/*
 * setFrame
 *
 * Seek to location within file, by framecount
 */
void OCTFile::setFrame( int requestedFrame )
{    
    int offset = 0;

    // If the request frame number is greater than the number of frames for the whole clip,
    // leave everything alone.
    if ( quint32(requestedFrame) >= ( totalNumberOfFrames + firstFrameNumber ) )
    {
        return;
    }

    // Prevent multiple access to the file handles
    readFrameLock.lock();
    hFile = nullptr;

    // Find the file which owns this frame
    for ( int i = indexList.count() - 1; i >= 0; i-- )
    {
        if ( requestedFrame > ( indexList.value( i ) + int(firstFrameNumber ) ) )
        {
            if ( i < hFileList.count() )
            {
                offset = requestedFrame - indexList.value( i ) - int(firstFrameNumber);
                hFile = hFileList.at( i + 1 );
                break;
            }
            else
            {
                // TBD: this leaves hFile == nullptr
                styledMessageBox::warning( tr( "File indexing failure:\nFrame does not match any currently loaded file " ) );
                readFrameLock.unlock();
                return;
            }
        }
    }

    // The first one it is
    if( !hFile )
    {
        hFile = hFileList.first();

        // Clips will not necessarily start at frame 0; the offset must be calculated
        // from the first frame of the clip
        offset = requestedFrame - int(firstFrameNumber);
        if( offset < 0 )
        {
            qDebug() << "Resetting offset to 0";
            offset = 0;
        }
    }

    resetFile();
    qint64 headerEnd = hFile->pos();

    hFile->seek( headerEnd + ( offset * framesize_bytes ) );
    readFrameLock.unlock();
}
