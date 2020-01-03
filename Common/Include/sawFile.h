/*
 * sawFile.h
 *
 * File and file system utilities
 *
 * Author: Dennis W. Jackson
 *
 * Copyright 2009 Sawtooth Labs, Inc.
 *
 */
#ifndef SAWFILE_H_
#define SAWFILE_H_

#include <QDataStream>
#include <QFile>
#include <QString>
#include <QTextStream>

#ifdef WIN32
#include <Windows.h>
#endif


class SawFile
{

public:
    static bool checkHash( QTextStream &in );
    static QByteArray computeHash( QString inputFile );

#ifdef WIN32
    static int getDiskFreeSpaceInGB( LPCWSTR drive );
#else
    static int getDiskFreeSpaceInGB( const char *path );
#endif

private:

    // This class exists only as a static utility class. Prevent creating,
    // destroying, copy, and assignment
    SawFile();
    ~SawFile();
    SawFile( const SawFile& obj );
    SawFile operator=( SawFile obj );

};

#endif // SAWFILE_H_
