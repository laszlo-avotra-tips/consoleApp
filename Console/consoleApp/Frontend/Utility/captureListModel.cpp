/*
 * captureListModel.cpp
 *
 * The capture database is a SQLite database used to reference
 * captured images and associated metadata (case information, etc.) for
 * quick cross-referencing and retrieval.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */

#include "captureListModel.h"
#include "userSettings.h"
#include "styledmessagebox.h"
#include <QDateTime>
#include <QDir>
#include "Utility/sessiondatabase.h"
#include "logger.h"

captureListModel* captureListModel::theDB{nullptr};
/*
 * constructor
 */
captureListModel::captureListModel(void)
    : QAbstractListModel()
{
}

/*
 * destructor
 */
captureListModel::~captureListModel()
{
    // walk the map and free memory
    QMapIterator<int, captureItem *> i(itemMap);
    while (i.hasNext()) {
        i.next();
        delete i.value();
        itemMap.remove( i.key() );
    }
}

/*
 * data()
 *
 * Accessor method used by Qt to return data from the model for
 * appropriate "roles". The index is used to determine the row
 * of the table. View widgets call this to construct their
 * representations of the model.
 */
QVariant captureListModel::data( const QModelIndex &index, int role ) const
{
    int row = index.row() + 1;

    // For sorting, just return the row number
    if( role == Qt::UserRole  )
    {
        return( row - 1 );
    }
    if( role == Qt::DisplayRole )
    {
        return( QVariant::fromValue( itemMap[ row ] ) );
    }
    if( role == Qt::ToolTipRole )
    {
        QString tipString( QString( "Time: %1\n" ).arg( itemMap[ row ]->getTimestamp() ) );
        tipString.append(  QString( "Tag: %1\n" ).arg(  itemMap[ row ]->getTag() ) );
        tipString.append(  QString( "Device: %1" ).arg( itemMap[ row ]->getDeviceName() ) );
        return( tipString );
    }
    else
    {
        return ( QVariant() );
    }

}

captureListModel &captureListModel::Instance()
{
    if(!theDB){
        theDB = new captureListModel();
    }
    return *theDB;
}


/*
 * addCapture()
 *
 * Add a new capture to the database, including
 * all associated data and file reference location.
 */
int captureListModel::addCapture(QString tag,
                                  uint timestamp,
                                  QString name,
                                  QString deviceName,
                                  bool isHighSpeed,
                                  int pixelsPerMm,
                                  float zoomFactor )
{

    QDateTime timeVal = QDateTime::fromTime_t(timestamp);
    // Find next available ID
    sessionDatabase db;
    int maxID = db.addCapture( tag, timestamp, name, deviceName, isHighSpeed, pixelsPerMm );
    if( maxID < 0 )
    {
        return -1;
    }

    // Update the model for the list view for the film strip of images
    QModelIndex root = index(0, 0, QModelIndex() );
    beginInsertRows( root, rowCount( root ), rowCount( root ) );

    // Add the item to the in memory cache
    captureItem *cap = new captureItem();
    cap->setdbKey( maxID );
    cap->setTag( tag );
    cap->setTimestamp( timeVal.toString( "yyyy-MM-dd HH:mm:ss" ) ); // use local time in the tool tip
    cap->setName( name );
    cap->setIdNumber( maxID );
    cap->setDeviceName( deviceName );
    cap->setPixelsPerMm( pixelsPerMm );
    cap->setZoomFactor( zoomFactor );
    itemMap.insert( maxID, cap );
    endInsertRows();
    return maxID;
}


/*
 * getLastCaptureId
 */
int captureListModel::getLastCaptureId( void )
{
    if( itemMap.empty() )
    {
        return -1;
    }
    QMap<int, captureItem *>::iterator i = itemMap.end();
    --i;
    return i.key();
}


/*
 * class captureItem implementation
 */

/*
 * loadImage()
 *
 * Give a type of image to load (specified in image filter as sector, etc.)
 * load it from disk and return a QImage representing it.
 */
QImage captureItem::loadImage( QString imageFilter )
{
    QStringList filters;
    caseInfo &info = caseInfo::Instance();
    filters << imageFilter;
    QString dirString = info.getStorageDir() + "/captures/"; // Set up the absolute path based on the session data.
    QDir thisDir( dirString );
    thisDir.setNameFilters( filters );
    
    // TBD: we filter to close to the exact name and grab the first thing we find
    if( thisDir.entryList().empty() )
    {
        return QImage();
    }
    else
    {
        return( QImage( thisDir.absoluteFilePath( thisDir.entryList().first() ) ) );
    }
}

/*
 * saveDecoratedImage
 *
 * Given a new image, replace the decorated image for the currently selected review image.
 * This is called via captureWidget, who is aware of the current captureItem.
 */
void captureItem::saveDecoratedImage(QImage newDecorated, QString imageFilter)
{
    caseInfo &info = caseInfo::Instance();
    QString dirString = info.getStorageDir() + "/captures/"; // Set up the absolute path based on the session data.

    QFile f( dirString + imageFilter );
    if( f.remove() )
    {
        // rotate the decorated image to match the display
        QMatrix m;
        m.rotate( 270 );
        QImage tmpImage = newDecorated.transformed( m );

        // Replace the decorated image.
        if( !tmpImage.save( f.fileName(), "PNG", 100 ) )
        {
            LOG( WARNING, "Image Capture: decorated image capture failed. Unable to save new decorated image." )
        }
    }
    else
    {
        LOG( DEBUG, "Image Capture: decorated image capture failed. Unable to remove original decorated image." )
    }
}
