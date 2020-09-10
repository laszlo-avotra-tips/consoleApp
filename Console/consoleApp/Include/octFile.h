/*
 * octFile.h
 *
 * Description:
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#ifndef OCTFILE_H_
#define OCTFILE_H_

#include <QDataStream>
#include <QObject>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>
#include "defaults.h"

class OCTFile : public QObject
{
    Q_OBJECT

public:
    // define which system is using this file
    enum FileAccessMode {
        Console,    // write-only access
        Player      // read-only access
    };
    // Changes in this structure must have changes made in writeFileHeader()
    // and writeNumFramesWritten() as well
    struct OctFileHeader_t
    {
        char magic[ 3 ];
        unsigned char formatVersion;
        unsigned short numLinesPerRevolution;
        quint32 numFramesWritten;
    };

    struct OctData_t
    {
        unsigned long  frameCount{0};
        unsigned long  timeStamp{0};
        unsigned short milliseconds{0};
        unsigned short encoderPosition{0};  // XXX: hijack for fast OCT direction bit
        uint8_t *acqData{nullptr};
        uint8_t *advancedViewIfftData{nullptr};         // used for Advanced View only
        uint8_t *advancedViewFftData{nullptr};         // used for Advanced View, data storage
        uint8_t *dispData{nullptr};        // used for display
        uint8_t *videoData{nullptr};       // High Speed only: video frame with appropriate rotation built in
    };

    OCTFile( unsigned short numLinesPerRev ); // constructor for the Console
    ~OCTFile( void );

    void open( void );

    QString getCurrentFullyQualifiedFileName( void ) { return currFullyQualifiedFileName; }
    QString getCurrentFileName( void ) { return currFileName; }

    void setStorageDir( QString dirName ) { storageDir = dirName; }
    void setNamePrefix( QString name ) { namePrefix = name + "-"; }
    void setFileId( QString id ) { fileId = id + "-"; }

    static bool checkHash( QTextStream &in );
    static QByteArray computeHash( QString inputFile );
    void setFrame( int frame );  // TBD: was private, changed to public for advance/rewind buttons

signals:
    void sendFileToKey( QString );

private:
    QFile *hFile;
    QDataStream outputStream;
    QString currFullyQualifiedFileName;
    QString currFileName;
    QString storageDir;
    QString namePrefix;
    QString fileId;
    unsigned int fileIteration;
    quint32 recordLength;
    quint32 framesWritten;
    OctFileHeader_t header;

    // playback
    QList<int> indexList;
    QList<QFile *> hFileList;
    int usPerFrame;
    quint32 totalNumberOfFrames;
    int framesize_bytes;
    quint32 firstTimestamp; // TBD: do jumps relative to timestamps in clips?
    quint32 lastTimestamp; // TBD
    quint32 firstFrameNumber;
    quint32 lastFrameNumber;
    QMutex readFrameLock;
    void processHeaders();
    void resetFile( void );
    QString fileFilter;
    QString playbackDir;

    QString genFullyQualifiedFileName( void );
    void writeNumFramesWritten( void );
    void writeFileHeader( void );
    void close( void );

    // Prevent default constructor, copy and assignment
    OCTFile( void );
    OCTFile( const OCTFile& obj );
    OCTFile operator=( OCTFile obj );
};

#endif // OCTFILE_H_
