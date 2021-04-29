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
#include "logger.h"
#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>
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

QImage *deviceSettings::getSelectedIcon() const
{
    return m_selectedIcon;
}

void deviceSettings::setSelectedIcon(QImage *selectedIcon)
{
    m_selectedIcon = selectedIcon;
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
        device->setALineLength_px(aLineLength);
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

    sm->setALineLength_px(dev->getALineLength_px());
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
    LOG1(deviceFile);

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
        QTextStream qts(&errMessage);
        int errLine;
        int errColumn;

        if( !doc->setContent( file, &errMessage, &errLine, &errColumn  ) )
        {
            retVal = false;
            qts << "Failed to parse XML file" << fileInfo.fileName() << errMessage << errLine << errColumn;
            LOG1(errMessage);
        }
    }

    if( retVal )
    {
        // read in data
        QDomElement root = doc->documentElement();

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
            QStringList fn = deviceFile.split(".");
            QString fn1 = fn[0] + "_Highlight.png";
            QString fn2 = fn[0] + "_Nohighlight.png";
            QImage *d1Img = new QImage;
            if( !d1Img->load( deviceFile.replace( DeviceDescriptionExtension, DeviceIconExtension, Qt::CaseInsensitive ) ) )
            {
                QString msg("Failed loading icon");
                LOG1(msg)
            }

            QImage *d2Img = new QImage;
            QImage *d3Img = new QImage;
            d2Img->load(fn1);
            d3Img->load(fn2);

            DeviceIconType deviceIcon{d3Img,d2Img};
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
                                     e.attribute( "torqueLimit", "25" ).toLatin1(),
                                     e.attribute( "torqueTime", "1" ).toLatin1(),
                                     e.attribute( "stallBlinking", "1" ).toLatin1(),

                                     e.attribute( "buttonMode", "0" ).toLatin1(),
                                     e.attribute( "measurementVersion", "1" ).toInt(),                                     
                                     e.attribute( "disclaimerText", InvestigationalDeviceWarning ),
                                     e.attribute( "deviceCRC","").toLatin1(),

                                     deviceIcon );    // d1Img
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

void device::setALineLength_px(int value)
{
    aLineLength_px = value;
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

bool deviceSettings::getIsDeviceSimulation() const
{
    return m_isDeviceSimulation;
}

void deviceSettings::setIsDeviceSimulation(bool isDeviceSimulation)
{
    m_isDeviceSimulation = isDeviceSimulation;
}

const QString &device::getSplitDeviceName() const
{
    return splitDeviceName;
}

int device::getALineLength_px() const
{
    return aLineLength_px;
}

float device::getImagingDepth_mm() const
{
    return imagingDepth_mm;
}
