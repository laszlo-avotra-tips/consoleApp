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
    clipItem(void) 
    {
        deviceName      = QString();
        tag             = QString();
        timestamp       = QString();
        name            = QString();
        dbKey           = -1;
        clipLength_s    = 0;
        catheterViewStr = "DistalToProximal";
    }
    ~clipItem() {}
    int getdbKey(void) {
        return dbKey;
    }
    void setdbKey(int key) {
        dbKey = key;
    }
    void setTag( QString tagStr )
    {
        tag = tagStr;
    }
    void setTimestamp( QString timeStampStr )
    {
        timestamp = timeStampStr;
    }
    void setCatheterView( QString str )
    {
        catheterViewStr = str;
    }
    void setName( QString capName )
    {
        name = capName;
    }
    void setDeviceName( QString devName )
    {
        deviceName = devName;
    }

    QString getTag()
    {
        return tag;
    }
    QString getTimestamp()
    {
        return timestamp;
    }
    void setLength( int length_ms )
    {
        // Add 500 ms and integer divide to round the result to the nearest second
        clipLength_s = ( length_ms + 500 ) / 1000;
    }
    QString getName()
    {
        return name;
    }
    int getLength( void )
    {
        return clipLength_s;
    }
    QString getCatheterView( void )
    {
        return catheterViewStr;
    }
    QString getDeviceName()
    {
        return deviceName;
    }
    QImage loadSectorThumbnail( QString nameSectorThumbnail ) {
        return( loadImage( ".thumb_" + nameSectorThumbnail + "*sector.png" ) );
    }

private:
    QImage loadImage(QString);

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
    int columnCount(const QModelIndex&) const {
        return(1);
    }
    QModelIndex parent(const QModelIndex &) const {
        return( QModelIndex() );
    }

    QVariant data(const QModelIndex &index, int role) const;

    // singleton
    static clipListModel & Instance(void);

    int addClipCapture(QString name,
                        uint timestamp,
                        QString catheterView,
                        QString deviceName);

    void updateClipInfo( int clipLength_ms );

    QList<clipItem *> getItemsByTag(QString tag);
    QList<clipItem *> getItemsByDate(QString date);
    QList<clipItem *> getAllItems(void) {
        return itemMap.values();
    }

    int getLastCaptureId( void );


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
    static clipListModel* theDB;
};
