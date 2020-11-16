/*
 * clipListModel.cpp
 *
 * A custom model for containing the clip (OCT Loops) and related data.  Used by
 * the filmstrip view in the main UI.
 *
 * Author: Chris White, Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */

#include "clipListModel.h"
#include "userSettings.h"
#include "defaults.h"
#include "styledmessagebox.h"
#include <QDateTime>
#include <QDir>
#include "Utility/sessiondatabase.h"

/*
 * constructor
 */
clipListModel::clipListModel(void)
    : QAbstractListModel()
{
    lastClipID = -1;
}

/*
 * destructor
 */
clipListModel::~clipListModel()
{
    // walk the map and free memory
    QMapIterator<int, clipItem *> i( itemMap );
    while( i.hasNext() )
    {
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
QVariant clipListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row() + 1;

    // For sorting, just return the row number
    if ( role == Qt::UserRole  ) {
        return( row - 1 );
    }
    if ( role == Qt::DisplayRole ) {
        return(QVariant::fromValue(itemMap[row]));
    }
    if ( role == Qt::ToolTipRole ) {
        QString tipString( QString( "Time: %1\n" ).arg( itemMap[row]->getTimestamp() ) );
        tipString.append( QString( "Tag: %1\n" ).arg( itemMap[row]->getTag() ) );
        tipString.append( QString( "Length: %1 s\n" ).arg( itemMap[ row ]->getLength() ) );
        tipString.append( QString( "Device: %1" ).arg( itemMap[row]->getDeviceName() ) );
        return( tipString );
    }
    else return ( QVariant() );

}


/*
 * addClipCapture()
 *
 * Add a new capture to the database, including
 * all associated data and file reference location.
 */
int clipListModel::addClipCapture( QString name,
                                   int timestamp,
                                   QString catheterView,
                                   QString deviceName,
                                   bool isHighSpeed )
{
    QDateTime timeVal = QDateTime::fromTime_t(timestamp);

    // Find next available ID
    sessionDatabase db;
    int maxID = db.addClipCapture( name, timestamp, catheterView, deviceName );

    if ( maxID < 0 ) {
        return -1;
    }

    lastClipID = maxID;

    // Update the model for the list view for the film strip of images
    QModelIndex root = index(0, 0, QModelIndex() );
    beginInsertRows( root, rowCount( root ), rowCount( root ) );

    // Add the item to the in-memory cache
    clipItem *clip = new clipItem();
    clip->setdbKey( maxID );
    clip->setTag( name );
    clip->setName( name );
    clip->setTimestamp( timeVal.toString("yyyy-MM-dd HH:mm:ss") ); // use local time in tool tips
    clip->setLength( 0 ); // length is set after the recording is finished
    clip->setCatheterView( catheterView );
    clip->setDeviceName( deviceName );
    itemMap.insert( maxID, clip );

    endInsertRows();

    return maxID;
}

/*
 * updateClipInfo()
 *
 * Update data about the clip that can only be known after
 * the clip is finished recording.
 */
void clipListModel::updateClipInfo( int clipLength_ms )
{
    // update the database
    sessionDatabase db;
    db.updateClipCapture( lastClipID, clipLength_ms );

    // update the in-memory cache
    clipItem *clip = new clipItem();
    clip = itemMap.value( lastClipID );
    clip->setLength( clipLength_ms );

    // Update the model for the list view for the film strip of images. Inserting
    // with the same key replaces the item in the list
    QModelIndex root = index(0, 0, QModelIndex() );
    beginInsertRows( root, rowCount( root ), rowCount( root ) );
    itemMap.insert( lastClipID, clip );
    endInsertRows();
}

/*
 * getLastCaptureId
 */
int clipListModel::getLastCaptureId( void )
{
    if ( itemMap.empty() ) {
        return -1;
    }
    QMap<int, clipItem *>::iterator i = itemMap.end();
    --i;
    return i.key();
}



/*
 * class clipItem implementation
 */

/*
 * loadImage()
 *
 * Give a type of image to load (specified in image filter as waterfall, sector, etc.)
 * load it from disk and return a QImage representing it.
 */
QImage clipItem::loadImage( QString imageFilter )
{
    QStringList filters;
    caseInfo &info = caseInfo::Instance();
    filters << imageFilter;
    QString dirString = info.getStorageDir() + "/clips/"; // Set up the absolute path based on the session data.
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
