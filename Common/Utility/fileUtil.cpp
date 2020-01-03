/*
 * util.cpp
 *
 * Utilities needed in various classes.
 *
 * Author: Chris White, Dennis Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "fileUtil.h"


/*
 * deleteDir()
 *
 * Recursively (!) delete a non-empty directory. Excludes deleting the top level
 * directory if excludeTopLevelDir is set to true. Set to false by default.
 */
bool fileUtil::deleteDir( QString dirPath, bool excludeTopLevelDir )
{
    QDir dir( dirPath );
    bool result = true;

    if( dir.exists( dirPath ) )
    {
        Q_FOREACH( QFileInfo info,
                   dir.entryInfoList( QDir::NoDotAndDotDot |
                                      QDir::System  |
                                      QDir::Hidden  |
                                      QDir::AllDirs |
                                      QDir::Files,
                                      QDir::DirsFirst ) )
        {

            if( info.isDir() )
            {
                result = deleteDir( info.absoluteFilePath() );
            }
            else
            {
                result = QFile::remove( info.absoluteFilePath() );
            }
            qDebug() << "Delete: " << info.absoluteFilePath() << "Delete result: " << result;
            if( !result )
            {
                return result;
            }
        }

        /*
         * Check the excludeParent flag, and skip the remove dir if the path matches the
         * parent directory.
         */
        if( excludeTopLevelDir && ( dirPath.compare( dirPath, Qt::CaseInsensitive ) == 0 ) )
        {
            result = true;
        }
        else
        {
            result = dir.rmdir( dirPath );
        }
    }

    return result;
}

/*
 * dirSize()
 *
 * Return the complete size of a directory and all subdirs and files.
 * Recursively. We have plenty of stack.
 */
qint64 fileUtil::dirSize( const QString &dirName )
{
    qint64 size = 0;

    QFileInfo fileInfo( dirName );
    if ( fileInfo.isDir() )
    {
        QDir dir( dirName );
        QFileInfoList list = dir.entryInfoList( QDir::Files | QDir::Dirs |  QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot );
        for ( int i = 0; i < list.size(); i++ )
        {
            QFileInfo fileInfovALUE = list.at( i );
            if( fileInfovALUE.isDir() )
            {
                size += dirSize( fileInfovALUE.absoluteFilePath() );
            }
            else
            {
                size += fileInfovALUE.size();
            }
        }
    }
    return size;
}

/*
 * searchFiletypeRecursive
 *
 * Recursively search for a file ending in the passed string. Returns a list of file paths
 * that end in the passed string.
 */
QFileInfoList fileUtil::searchFiletypeRecursive( const QString &sDir, QString filter )
{
    QDir dir(sDir);
    QFileInfoList list = dir.entryInfoList();
    QFileInfoList resultList;
    for( int iList = 0; iList < list.count(); iList++ )
    {
        QFileInfo info = list[ iList ];

        QString sFilePath = info.filePath();
        if( info.isDir() )
        {
            // recursive
            if( ( info.fileName()!= ".." ) && ( info.fileName()!= "." ) )
            {
                resultList << searchFiletypeRecursive( sFilePath, filter );
            }
        }
        else
        {
            if( info.filePath().endsWith( filter, Qt::CaseInsensitive ) )
            {
                resultList << info.filePath();
            }
        }
    }
    return resultList;
}
