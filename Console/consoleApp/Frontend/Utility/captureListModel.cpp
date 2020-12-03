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

captureListModel* captureListModel::m_instance{nullptr};
/*
 * constructor
 */
captureListModel::captureListModel(void)
    : QAbstractListModel()
{
}

bool captureListModel::getIsSelected() const
{
    return m_isSelected;
}

void captureListModel::setIsSelected(bool isSelected)
{
    m_isSelected = isSelected;
}

int captureListModel::getRowOffset() const
{
    return m_rowOffset;
}

void captureListModel::setRowOffset(int rowOffset)
{
    m_rowOffset = rowOffset;
}

int captureListModel::getSelectedRow() const
{
    return m_selectedRow;
}

void captureListModel::setSelectedRow(int selectedRow)
{
    m_selectedRow = selectedRow;
}

void captureListModel::reset()
{
    // walk the map and free memory
    QMapIterator<int, captureItem *> i(itemMap);
    while (i.hasNext()) {
        i.next();
        delete i.value();
        itemMap.remove( i.key() );
    }
    m_selectedRow = -1;
    setRowOffset(0);
}

/*
 * destructor
 */
captureListModel::~captureListModel()
{
    reset();
    LOG1("~")
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
    else
    {
        return ( QVariant() );
    }
}

captureListModel &captureListModel::Instance()
{
    if(!m_instance){
        LOG1(m_instance)
        m_instance = new captureListModel();
    }
    return *m_instance;
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
                                  int pixelsPerMm,
                                  float zoomFactor )
{

    QDateTime timeVal = QDateTime::fromTime_t(timestamp);
    // Find next available ID
    sessionDatabase db;
    db.initDb();
    int maxID = db.addCapture( tag, timestamp, name, deviceName, pixelsPerMm );
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

int captureListModel::countOfCapuredItems() const
{
    return itemMap.size();
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
