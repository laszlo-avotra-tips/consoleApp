/*
 * captureListModel.h
 *
 * A custom model for containing the image captures and related data.  Used by
 * the filmstrip view in the main UI.
 *
 * This file also declares the interface for a QAbstractListModel,
 * providing a standard model to interface between the database
 * and the QListView style widgets.
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 * Author: Chris White
 */
#pragma once

#include <QAbstractListModel>
#include <QImage>
#include <QObject>
#include "defaults.h"

class captureItem 
{
public:
    captureItem( void )
    {
        deviceName  = QString();
        tag         = QString();
        timestamp   = QString();
        name        = QString();
        idNumber    = 0;
        pixelsPerMm = 1;
        zoomFactor  = 1.0;
        dbKey       = -1;
    }
    ~captureItem() {}

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
    void setName( QString capName )
    {
        name = capName;
    }
    void setIdNumber( int num )
    {
        idNumber = num;
    }
    void setDeviceName( QString devName )
    {
        deviceName = devName;
    }
    void setPixelsPerMm( int capPixelsPerMm )
    {
        pixelsPerMm = capPixelsPerMm;
    }
    void setZoomFactor( float zoom )
    {
        zoomFactor = zoom;
    }

    QString getTag()
    {
        return tag;
    }
    QString getTimestamp()
    {
        return timestamp;
    }
    QString getName()
    {
        return name;
    }
    int getIdNumber()
    {
        return idNumber;
    }
    QString getDeviceName()
    {
        return deviceName;
    }
    int getPixelsPerMm()
    {
        return pixelsPerMm;
    }
    float getZoomFactor()
    {
        return zoomFactor;
    }

    // XXX: do not like wildcarding based on the base name
    QImage loadSector( QString name ) { return( loadImage( name + "*" + ".png" ) ); }
    QImage loadSectorThumbnail( QString name ) { return( loadImage( ".thumb_" + name + "*" + ".png" ) ); }
    QImage loadDecoratedImage( QString name ) { return( loadImage( name + "*" + ".png" ) ); }
    void replaceDecoratedImage( QImage p ) { saveDecoratedImage( p, name + ".png" ); }

private:
    QImage loadImage(QString);
    void saveDecoratedImage(QImage,QString);

    QString deviceName;
    QString tag;
    QString timestamp;
    QString name;
    int idNumber;
    int pixelsPerMm;
    float zoomFactor;

    int dbKey;
};

// Notify Qt the captureItem is going to be used as a custom variant
Q_DECLARE_METATYPE(captureItem *)

/*
 *
 */
class captureListModel : public QAbstractListModel
{
    Q_OBJECT

public:
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
    static captureListModel & Instance(void);

    int addCapture( QString tag,
                    uint timestamp,
                    QString name,
                    QString deviceName,
                    int pixelsPerMm,
                    float zoomFactor );

    QList<captureItem *> getItemsByTag(QString tag);
    QList<captureItem *> getItemsByDate(QString date);
    QList<captureItem *> getAllItems(void) const
    {
        return itemMap.values();
    }

    int getLastCaptureId(void);
    ~captureListModel();

    int getSelectedRow() const;
    void setSelectedRow(int selectedRow);

signals:

    void warning( QString );
    void error( QString );

public slots:

private:
    // Hide ctor
    captureListModel();

    // Hide copy and assign operator
    captureListModel(captureListModel const &);
    captureListModel& operator = (captureListModel const &);

    // In memory cache of all captures keyed by id.
    // This is so we can do a two-level search for speedup:
    // Lookup based on search criteria in database, get id
    // of each result lookup in here to get the actual data item.
    // This needs to be rebuilt at load time for playback.
    QMap<int, captureItem *> itemMap;

    int m_selectedRow;

    static captureListModel* theDB;
};
