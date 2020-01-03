/*
 * fileUtil.h
 *
 * File/directory based functions that are needed across various classes.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */
#ifndef HOMESCREEN_FILEUTIL_H
#define HOMESCREEN_FILEUTIL_H

#include <QString>
#include <QFileInfoList>

class fileUtil
{
public:
    static bool deleteDir( QString, bool excludeTopLevelDir = false );
    static QFileInfoList searchFiletypeRecursive( const QString &sDir, QString filter );
    static qint64 dirSize( const QString &dir);
};

#endif // HOMESCREEN_FILEUTIL_H
