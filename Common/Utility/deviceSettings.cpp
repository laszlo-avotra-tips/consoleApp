/*
 * deviceSettings.cpp
 *
 * Device specific settings object. Contains physical and
 * imaging characteristics for a particular model of imaging
 * catheter.
 *
 * Author: Chris White, Bjarne Christensen
 *
 * Copyright (c) 2010-2017 Avinger, Inc.
 */
#include <QDebug>
#include "deviceSettings.h"
#include "defaults.h"
//#include "util.h"
#include "logger.h"
#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QSettings>
#include <QMessageBox>
#include <logger.h>
#include "Utility/userSettings.h"
#include "signalmodel.h"


#define MASK_STEP_SIZE 1

/*
 * constructor
 */
deviceSettings::deviceSettings(void) : QObject()
{
    settings      = NULL;
    currentDevice = -1;     // No device selected
}

/*
 * destructor
 */
deviceSettings::~deviceSettings() 
{
    device *dev;
    foreach( dev, deviceList ) 
    {
        if( dev != NULL )
        {
            delete dev;
        }
    }
}

/*
 * init
 *
 * Load all of the devices found in the System area. Return the number of loaded devices.
 */
deviceSettings &deviceSettings::Instance()
{
    static deviceSettings theSettings;
    return theSettings;
}

int deviceSettings::init( void )
{
    int  numDevicesLoaded = 0;
    QDir deviceDir( DevicesPath );
    qDebug() << "Device directory: " << DevicesPath;
    QFileInfoList list = deviceDir.entryInfoList( QStringList( "*.xml" ) );

    for( int i = 0; i < list.size(); i++ )
    {
        if( loadDevice( DevicesPath + "/" + list.at( i ).fileName() ) )
        {
            numDevicesLoaded++;
        }
    }
    auto& settings = userSettings::Instance();
    const float depth = settings.getImagingDepth_mm() / 2.0f;
    const int aLineLength = settings.getALineLength_px() / 2;
    LOG3(numDevicesLoaded, depth, aLineLength)
    for(auto device : deviceList){
        device->setImagingDepth_mm(depth);
        device->setALineLengthNormal_px(aLineLength);
    }

    return numDevicesLoaded;
}

void deviceSettings::setCurrentDevice( int devIndex )
{
    qDebug() << "* DeviceSettings - Current device changed";
    currentDevice = devIndex;
    deviceSettings::adjustMaskSize( 0 );

    auto* sm = SignalModel::instance();
    const auto* dev = current();

    sm->setALineLengthNormal_px(dev->getALineLengthNormal_px());
    sm->setStandardDepth_mm(dev->getImagingDepth_mm());
    sm->setInternalImagingMask_px(dev->getInternalImagingMask_px());
    sm->setCatheterRadius_um(dev->getCatheterRadius_um());
}

device *deviceSettings::current()
{
    if ( currentDevice >= 0 )
    {
        return deviceList.at( currentDevice );
    }
    else
    {
        return NULL;
    }
}

const QString& deviceSettings::getCurrentDeviceName() const
{
    if ( currentDevice >= 0 )
    {
        return deviceList.at( currentDevice )->getDeviceName();
    }
    else
    {
        return m_unknownDeviceName;
    }
}

const QString &deviceSettings::getCurrentSplitDeviceName() const
{
    if ( currentDevice >= 0 )
    {
        return deviceList.at( currentDevice )->getSplitDeviceName();
    }
    else
    {
        return m_unknownDeviceName;
    }
}

QString deviceSettings::getCurrentDeviceTitle() const
{
    QString title;
    if ( currentDevice >= 0 )
    {
        auto device = deviceList.at( currentDevice );
        auto deviceName = device->getDeviceName();
        QStringList words = deviceName.split(" ");
        if(words.size() == 3){
            title = QString("%1 %2").arg(words[0]).arg(words[1]);
        }
    }
    return title;
}

/*
 * loadDevice
 *
 * Read individual device XML file and add it to the list in Device Select.
 */
bool deviceSettings::loadDevice( QString deviceFile )
{
    int testno = 0;
    //errorHandler &err = errorHandler::Instance();

    qDebug() << "Loading device from:" << deviceFile;

//    device::DeviceType speedType = device::LowSpeed;    // to make compiler happy that it is initialized
    bool retVal = false;

    // open the file
    QFile        *file = new QFile( deviceFile );
    QDomDocument *doc  = new QDomDocument("");
    QFileInfo    fileInfo;

    // do not display the file path in the warning message, but use the full path for the LOG message
    if( ( file == NULL ) || ( doc == NULL ) )
    {
        //err.warn( "Failed to open device XML file." );
        LOG( WARNING, QString( "Failed to open device XML file %1." ).arg( deviceFile ) );
    }
    else if( !file->open( QFile::ReadOnly | QFile::Text ) )
    {
        //err.warn( "Failed to open device XML file." );
        LOG( WARNING, QString( "Failed to open device XML file %1." ).arg( deviceFile ) );
    }
    else
    {
        retVal = true;

        // get the filename from the filepath
        fileInfo = file->fileName();
        QString errMessage;
        int errLine;
        int errColumn;

        if( !doc->setContent( file, &errMessage, &errLine, &errColumn  ) )
        {
            retVal = false;
            qDebug() << "Failed to parse XML file" << fileInfo.fileName() << errMessage << errLine << errColumn;
            //err.warn( QString( "Failed to parse XML file %1." ).arg( fileInfo.fileName() ) );
        }
    }
    testno++;
    if (retVal != true) {
        qDebug() << "test: " << testno;
    }
//    // do not continue if the device XML version is wrong
//    if( retVal )
//    {
//        if( !checkVersion( doc ) )
//        {
//            retVal = false;
//            //err.warn( QString( "Failed to load device %1\nReason: Version mismatch" ).arg( fileInfo.fileName() ) );
//        }
//    }
//    testno++;
//    if (retVal != true) qDebug() << "test: " << 2;

    if( retVal )
    {
        // read in data
        QDomElement root = doc->documentElement();

//        QDomNode n = root.firstChild();
//        QDomElement e = n.toElement();
        QDomElement e = root.toElement();
        // read attributes for device
        if( e.tagName() == "device" )
        {
            /*
             * Load the device icon file; these are published along side the
             * device XML files.  If it cannot be loaded use the built-in
             * unknown device. This ensures that the device will be on the
             * selection list.
             */
            QImage *d1Img = new QImage;
            if( !d1Img->load( deviceFile.replace( DeviceDescriptionExtension, DeviceIconExtension, Qt::CaseInsensitive ) ) )
            {
                qDebug() << "Failed loading icon";
                //d1Img->load( ":/octConsole/Frontend/Resources/unknowndev.jpg" );
            }

            device *d1 = new device( e.attribute( "deviceName", "" ),
                                     e.attribute( "type", "" ).toLatin1(),
                                     e.attribute( "devicePropVersion","").toLatin1(),
                                     e.attribute( "sledFwMinVersion","").toLatin1(),
                                     e.attribute( "ifFwMinVersion", "").toLatin1(),
                                     e.attribute( "catheterLength", "" ).toInt(),
                                     e.attribute( "internalImagingMask_px", "" ).toInt(),
                                     e.attribute( "catheterRadius_um", "" ).toInt(),
                                     e.attribute( "biDirectional", "0" ).toInt(),
                                     e.attribute( "numberOfSpeeds", "1" ).toInt(),
                                     e.attribute( "revolutionsPerMin1", "600" ).toInt(),
                                     e.attribute( "revolutionsPerMin2", "800" ).toInt(),
                                     e.attribute( "revolutionsPerMin3", "1000" ).toInt(),
                                     e.attribute( "defaultSpeedIndex", "1").toInt(),
                                     e.attribute( "clockingEnabled", "1" ).toInt(),
                                     e.attribute( "clockingGain", "" ).toLatin1(),
                                     e.attribute( "clockingOffset", "" ).toLatin1(),
                                     e.attribute( "torqueLimit", "2.5" ).toLatin1(),
                                     e.attribute( "timeLimit", "1" ).toLatin1(),
                                     e.attribute( "measurementVersion", "1" ).toInt(),
                                     e.attribute( "disclaimerText", InvestigationalDeviceWarning ),
                                     e.attribute( "deviceCRC","").toLatin1(),
                                     d1Img );    // d1Img
            deviceList.append( d1 );
        }
        // close the XML file
        file->close();
    }

    return retVal;
}

/*
 * getFileSystemSafeDeviceName()
 *
 * Replace unsafe characters in the device name with underscores.
 */
QString deviceSettings::getFileSystemSafeDeviceName( void )
{
    QString     deviceName = getCurrentDeviceName();

    QStringList badChars;
    badChars << "\"" << "*" << "/" << ":" << "<" << ">"
             << "?" << "\\" << "[" << "]" << "|";

    for( int i = 0; i < badChars.size(); i++ )
    {
        deviceName.replace( badChars.at( i ), QString( "_" ) );
    }

    return deviceName;
}

/*
 * checkVersion
 *
 * Check the file's Device Xml Schema Version, and return true if the correct version.
 */
bool deviceSettings::checkVersion( QDomDocument *doc )
{
    bool retVal = false;
    QDomElement root = doc->documentElement();
    if( ( root.tagName() == "properties" ) && ( root.attribute( "version", "unknown version" ) == DeviceXmlSchemaVersion ) )
    {
        retVal = true;
    }

    return retVal;
}

/*
 *  Change the mask size for engineering tests
 */
void deviceSettings::adjustMaskSize( int step )
{
    int index = getCurrentDevice();
    int newMask = deviceAt( index )->getInternalImagingMask_px();
    deviceAt( index )->setInternalImagingMask_px( newMask + (step * MASK_STEP_SIZE));
    emit displayMask( newMask );
    qDebug() << "New Mask: " << deviceAt(index)->getInternalImagingMask_px();
}

QString device::formatDeviceName(const QString &name)
{
    QString retVal;
    QStringList words = name.split(" ");
    int i = 0;
    for(const auto& word : words){
        switch(i){
        case 0:
            retVal = QString(" %1").arg(word);
            break;
        case 1:
            retVal += QString(" %1\n").arg(word);
            break;
        default:
            retVal += QString(" %1").arg(word);
            break;
        }
        ++i;
    }

    return retVal;
}

void device::setImagingDepth_mm(float value)
{
    imagingDepth_mm = value;
}

void device::setALineLengthNormal_px(int value)
{
    aLineLengthNormal_px = value;
}

QByteArray device::getDevicePropVersion() const
{
    return devicePropVersion;
}

void device::setDevicePropVersion(const QByteArray &value)
{
    devicePropVersion = value;
}

QByteArray device::getSledFwMinVersion() const
{
    return sledFwMinVersion;
}

void device::setSledFwMinVersion(const QByteArray &value)
{
    sledFwMinVersion = value;
}

QByteArray device::getIfFwMinVersion() const
{
    return ifFwMinVersion;
}

void device::setIfFwMinVersion(const QByteArray &value)
{
    ifFwMinVersion = value;
}

int device::getDefaultSpeedIndex() const
{
    return defaultSpeedIndex;
}

void device::setDefaultSpeedIndex(int value)
{
    defaultSpeedIndex = value;
}

QByteArray device::getDeviceCRC() const
{
    return deviceCRC;
}

void device::setDeviceCRC(const QByteArray &value)
{
    deviceCRC = value;
}

int device::getNumberOfSpeeds() const
{
    return numberOfSpeeds;
}

void device::setNumberOfSpeeds(int value)
{
    numberOfSpeeds = value;
}

bool deviceSettings::getIsSimulation() const
{
    return m_isSimulation;
}

void deviceSettings::setIsSimulation(bool isSimulation)
{
    m_isSimulation = isSimulation;
}

const QString &device::getSplitDeviceName() const
{
    return splitDeviceName;
}

int device::getALineLengthNormal_px() const
{
    return aLineLengthNormal_px;
}

float device::getImagingDepth_mm() const
{
    return imagingDepth_mm;
}
