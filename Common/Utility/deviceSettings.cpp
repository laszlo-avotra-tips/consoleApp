/*
 * deviceSettings.cpp
 *
 * Device specific settings object. Contains physical and
 * imaging characteristics for a particular model of imaging
 * catheter.
 *
 * Author: Chris White, Bjarne Christensen
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */
#include <QDebug>
#include "deviceSettings.h"
#include "defaults.h"
#include "util.h"
#include "logger.h"
#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QSettings>
#include <QMessageBox>
#include "logger.h"

deviceSettings* deviceSettings::theSettings{nullptr};

// singleton
deviceSettings & deviceSettings::Instance()
{
    if(!theSettings){
        theSettings = new deviceSettings();
    }
    return *theSettings;
}

/*
 * constructor
 */
deviceSettings::deviceSettings(void) : QObject()
{
    settings      = nullptr;
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
        if( dev )
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
int deviceSettings::init( void )
{
    int  numDevicesLoaded = 0;
    QDir deviceDir( DevicesPath );
    QFileInfoList list = deviceDir.entryInfoList( QStringList( "*.xml" ) );

    for( int i = 0; i < list.size(); i++ )
    {
        if( loadDevice( DevicesPath + "/" + list.at( i ).fileName() ) )
        {
            numDevicesLoaded++;
        }
    }

    return numDevicesLoaded;
}

/*
 * loadDevice
 *
 * Read individual device XML file and add it to the list in Device Select.
 */
bool deviceSettings::loadDevice( QString deviceFile )
{
    errorHandler &err = errorHandler::Instance();

    bool retVal = false;

    // open the file
    QFile        *file = new QFile( deviceFile );
    QDomDocument *doc  = new QDomDocument("");
    QFileInfo    fileInfo;

    // do not display the file path in the warning message, but use the full path for the LOG message
    if( ( !file ) || ( !doc ) )
    {
        err.warn( "Failed to open device XML file." );
        LOG( WARNING, QString( "Failed to open device XML file %1." ).arg( deviceFile ) )
    }
    else if( !file->open( QFile::ReadOnly | QFile::Text ) )
    {
        err.warn( "Failed to open device XML file." );
        LOG( WARNING, QString( "Failed to open device XML file %1." ).arg( deviceFile ) )
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
            err.warn( QString( "Failed to parse XML file %1." ).arg( fileInfo.fileName() ) );
        }
    }

    // do not continue if the device XML version is wrong
    if( retVal )
    {
        if( !checkVersion( doc ) )
        {
            retVal = false;
            err.warn( QString( "Failed to load device %1\nReason: Version mismatch" ).arg( fileInfo.fileName() ) );
        }
    }

    if( retVal )
    {
        // read in data
        QDomElement root = doc->documentElement();

        QDomNode n = root.firstChild();
        QDomElement e = n.toElement();

        // read attributes for device
        if( e.tagName() == "device" )
        {
            device::DeviceType speedType{device::HighSpeed};
            QString type = e.attribute( "deviceType", "" );

            // must compare strings and assign to the enumerated type
            // comment that compare returns 0 if matched.
            if( !QString::compare( type, "LowSpeed" ) )
            {
                speedType = device::LowSpeed;
            }
            else if( !QString::compare( type, "HighSpeed" ) )
            {
                speedType = device::HighSpeed;
            }
            else if( !QString::compare( type, "Ocelaris" ) )
            {
                 speedType = device::Ocelaris;
            }
            else
            {
                retVal = false;
                err.warn( tr( "A device XML file has been altered and will not work." ) );
            }

            // set to false in case this attribute is undefined (Low Speed devices)
            bool isClockingEnabledByDefault = false;

            if( retVal && ((speedType == device::HighSpeed) || (speedType == device::Ocelaris)) )
            {
                // High Speed Devices must have clocking attributes defined, or else they
                // will prompt an warning message and be unavailable.
                if( !e.hasAttribute( "clockingEnabledByDefault" ) ||
                    !e.hasAttribute( "clockingGain" ) ||
                    !e.hasAttribute( "clockingOffset" ) )
                {
                    retVal = false;
                    QString msg = QString( tr( "%1\nThis High Speed device does not have proper clocking attributes and will not work." ) ).arg( fileInfo.fileName() );
                    err.warn( msg );
                    LOG( WARNING, msg )
                }
                else if( !e.hasAttribute( "torqueLimit" ) ||
                         !e.hasAttribute( "timeLimit" ) )
                {
                    retVal = false;
                    QString msg = QString( tr( "%1\nThis High Speed device does not have proper torque attributes and will not work." ) ).arg( fileInfo.fileName() );
                    err.warn( msg );
                    LOG( WARNING, msg )
                }
                else // valid high speed device profile with clocking parameters
                {
                    // parse the default clocking mode from string to bool
                    QString defaultClockingMode = e.attribute( "clockingEnabledByDefault", "" );
                    if( !QString::compare( defaultClockingMode, "true", Qt::CaseInsensitive ) )
                    {
                        isClockingEnabledByDefault = true;
                    }
                    else if( !QString::compare( defaultClockingMode, "false", Qt::CaseInsensitive ) )
                    {
                        isClockingEnabledByDefault = false;
                    }
                    else
                    {
                        retVal = false;
                    }

                    // range check clocking gain
                    int tmpGain = e.attribute( "clockingGain", "0" ).toInt(); // store the value read in temporarily
                    if( tmpGain < 1 || tmpGain > 255 )
                    {
                        retVal = false;
                    }

                    // range check clocking offset
                    int tmpOffset = e.attribute( "clockingOffset", "0" ).toInt(); // store the value read in temporarily
                    if( tmpOffset < 1 || tmpOffset > 999 )
                    {
                        retVal = false;
                    }

                    // range check torque
                    float tmpTorque = e.attribute( "torqueLimit", "0" ).toFloat(); // store the value read in temporarily
                    if( tmpTorque < 1.0f || tmpTorque > 4.5f )
                    {
                        retVal = false;
                    }

                    // range check torque limit timeout
                    int tmpLimit = e.attribute( "timeLimit", "0" ).toInt(); // store the value read in temporarily
                    if( tmpLimit < 1 || tmpLimit > 15 )
                    {
                        retVal = false;
                    }

                    if( !retVal )
                    {
                        QString msg = QString( tr( "%1\nThis High Speed device does not have proper attributes and will not work." ) ).arg( fileInfo.fileName() );
                        err.warn( msg );
                        LOG( WARNING, msg )
                    }
                }
            }

            // Only add the valid devices
            if( retVal )
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
                    d1Img->load( ":/octConsole/Frontend/Resources/unknowndev.jpg" );
                }

                /*
                 * Compute number of A-lines from revolutionsPerMin
                 * A-lines = (1000*1200) / revsPerMin (at 25kHz laser)
                 * Reduce to closest mod16 less than number - 4
                 */

                int aLines;
                int revsPerMin;
                int temp;

                revsPerMin = e.attribute( "revolutionsPerMin", "" ).toInt();
                if( revsPerMin != 0)
                {
                    temp = ((1000*1200) / revsPerMin) - 4;
                    aLines = temp - temp%16;
                }
                else        // OCELOT is set to 0 RPM
                {
                    aLines = e.attribute( "linesPerRevolution_cnt", "" ).toInt();
                }

                qDebug() << "RPM: " << revsPerMin << ", Number of A-Lines: " << aLines;

                device *d1 = new device( e.attribute( "deviceName", "" ),
                                         e.attribute( "internalImagingMask_px", "" ).toInt(),
                                         e.attribute( "catheterRadius_um", "" ).toInt(),
                                         aLines,
                                         e.attribute( "revolutionsPerMin", "" ).toInt(),
                                         e.attribute( "aLineLengthNormal_px", "" ).toInt(),
                                         e.attribute( "aLineLengthDeep_px", "" ).toInt(),
                                         e.attribute( "imagingDepthNormal_mm", "" ).toFloat(),
                                         e.attribute( "imagingDepthDeep_mm", "" ).toFloat(),
                                         isClockingEnabledByDefault,
                                         e.attribute( "clockingGain", "" ).toLatin1(),
                                         e.attribute( "clockingOffset", "" ).toLatin1(),
                                         e.attribute( "torqueLimit", "" ).toLatin1(),
                                         e.attribute( "timeLimit", "" ).toInt(),
                                         e.attribute( "limitBlinkEnabled", "-1" ).toInt(),
                                         e.attribute( "measurementVersion", "0" ).toInt(),
                                         e.attribute( "Speed1", "0" ).toLatin1(),
                                         e.attribute( "Speed2", "0" ).toLatin1(),
                                         e.attribute( "Speed3", "0" ).toLatin1(),
                                         e.attribute( "disclaimerText", InvestigationalDeviceWarning ),
                                         speedType,
                                         d1Img );

                deviceList.append( d1 );
            }
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
