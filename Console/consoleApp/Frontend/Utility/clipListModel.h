/*
 * clipListModel.h
 *
 * A custom model for containing the clip (OCT Loops) and related data.  Used by
 * the filmstrip view in the main UI.
 *
 * This file also declares the interface for a QAbstractListModel,
 * providing a standard model to interface between the database
 * and the QListView style widgets.
 *
 * Author: Chris White, Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */

#pragma once

#include <QAbstractListModel>
#include <QImage>
#include <QObject>

/*
 * clipItem
 */
class clipItem
{
public:
    clipItem(void):dbKey(-1),clipLength_s(0)
    {
    }
    ~clipItem() {}
    int getdbKey(void) {
        return dbKey;
    }
    void setdbKey(int key) {
        dbKey = key;
    }
    void setTag( const QString& tagStr )
    {
        tag = tagStr;
    }
    void setTimestamp( const QString& timeStampStr )
    {
        timestamp = timeStampStr;
    }
    void setCatheterView( const QString& str )
    {
        catheterViewStr = str;
    }
    void setName( const QString& capName )
    {
        name = capName;
    }
    void setDeviceName( const QString& devName )
    {
        deviceName = devName;
    }

    QString getTag() const
    {
        return tag;
    }
    QString getTimestamp() const
    {
        return timestamp;
    }
    void setLength( int length_ms )
    {
        // Add 500 ms and integer divide to round the result to the nearest second
        clipLength_s = ( length_ms + 500 ) / 1000;
    }
    QString getName() const
    {
        return name;
    }
    int getLength( void ) const
    {
        return clipLength_s;
    }
    QString getCatheterView( void ) const
    {
        return catheterViewStr;
    }
    QString getDeviceName() const
    {
        return deviceName;
    }
//    QImage loadSectorThumbnail( QString name ) {
//        return( loadImage( ".thumb_" + name + "*sector.png" ) );
//    }

private:
//    QImage loadImage(QString);

    QString deviceName;
    QString tag;
    QString timestamp;
    QString name;

    int dbKey;
    int clipLength_s;
    QString catheterViewStr;
};

// Notify Qt the clipItem is going to be used as a custom variant
Q_DECLARE_METATYPE(clipItem *)

/*
 * clipListModel
 */
class clipListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ~clipListModel();
    int rowCount(const QModelIndex &) const {
        int count = itemMap.count();
        return(count);
    }
    int columnCount(const QModelIndex) const {
        return(1);
    }
    QModelIndex parent(const QModelIndex &) const {
        return( QModelIndex() );
    }

    QVariant data(const QModelIndex &index, int role) const;

    // singleton
    static clipListModel & Instance(void) {
        static clipListModel theDB;
        return theDB;
    }

    int addClipCapture(QString name,
                        int timestamp,
                        QString catheterView,
                        QString deviceName , bool);

    void updateClipInfo( int clipLength_ms );

    QList<clipItem *> getItemsByTag(QString tag);
    QList<clipItem *> getItemsByDate(QString date);
    QList<clipItem *> getAllItems(void) {
        return itemMap.values();
    }
    int countOfClipItems() const;

    int getLastCaptureId( void );


    int getRowOffset() const;
    void setRowOffset(int rowOffset);

    int getSelectedRow() const;
    void setSelectedRow(int selectedRow);
    void reset();

    QString getOutDirPath() const;
    void setOutDirPath(const QString &outDirPath);

    QString getPlaylistThumbnail() const;
    void setPlaylistThumbnail(const QString &playlistThumbnail);

    int getCurrentLoopNumber() const;
    void setCurrentLoopNumber(int currentLoopNumber);

    QString getThumbnailDir() const;
    void setThumbnailDir(const QString &thumbnailDir);

signals:

    void warning( QString );
    void error( QString );

public slots:

private:
    // Hide ctor
    clipListModel();

    // Hide copy and assign operator
    clipListModel(clipListModel const &);
    clipListModel& operator = (clipListModel const &);

    // used to keep track of the last entry to allow updates when the recording finishes
    int lastClipID;

    // In memory cache of all captures keyed by id.
    // This is so we can do a two-level search for speedup:
    // Lookup based on search criteria in database, get id
    // of each result lookup in here to get the actual data item.
    // This needs to be rebuilt at load time for playback.
    QMap<int, clipItem *> itemMap;

    int m_selectedRow{-1};
    int m_rowOffset{0};

    QString m_outDirPath;
    QString m_thumbnailDir;
    QString m_playlistThumbnail;
    int m_currentLoopNumber{0};

};
