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

    return numDevicesLoaded;
}

void deviceSettings::setCurrentDevice( int devIndex )
{
    qDebug() << "* DeviceSettings - Current device changed";
    currentDevice = devIndex;
    deviceSettings::adjustMaskSize( 0 );
//lcv    emit deviceChanged( devIndex );
    emit deviceChanged( );
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

    device::DeviceType speedType = device::LowSpeed;    // to make compiler happy that it is initialized
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
    if (retVal != true) qDebug() << "test: " << testno;
    // do not continue if the device XML version is wrong
    if( retVal )
    {
        if( !checkVersion( doc ) )
        {
            retVal = false;
            //err.warn( QString( "Failed to load device %1\nReason: Version mismatch" ).arg( fileInfo.fileName() ) );
        }
    }
    testno++;
    if (retVal != true) qDebug() << "test: " << testno;

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

            // match strings and assign to enumerated type
            // match returns 0
            if( !QString::compare( type, "LowSpeed" ) )
            {
                speedType = device::LowSpeed;
            }
            else if( !QString::compare( type, "HighSpeed") )
            {
                speedType = device::HighSpeed;
            }
            else if( !QString::compare( type, "Ocelaris") )
            {
                speedType = device::Ocelaris;
            }
            else
            {
                retVal = false;
            }
            testno++;
            if (retVal != true) qDebug() << "test: " << testno;
            // range check clocking gain
            int tmpGain = e.attribute( "clockingGain", "0" ).toInt(); // store the value read in temporarily
            if( tmpGain < 1 || tmpGain > 255 )
            {
                retVal = false;
            }
            testno++;
            if (retVal != true) qDebug() << "test: " << testno;

            // range check clocking offset
            int tmpOffset = e.attribute( "clockingOffset", "0" ).toInt(); // store the value read in temporarily
            if( tmpOffset < 1 || tmpOffset > 999 )
            {
                retVal = false;
            }
            testno++;
            if (retVal != true) qDebug() << "test: " << testno;

            if( !retVal )
            {
                QString msg = QString( tr( "%1\nThis High Speed device does not have proper clocking attributes and will not work." ) ).arg( fileInfo.fileName() );
                //err.warn( msg );
                LOG( WARNING, msg );
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

                auto fn = deviceFile.replace( DeviceDescriptionExtension, DeviceIconExtension, Qt::CaseInsensitive);

                LOG1(fn)

                if( !d1Img->load(fn) )
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
                                         e.attribute( "clockingEnabled", "1" ).toInt(),
                                         e.attribute( "clockingGain", "" ).toLatin1(),
                                         e.attribute( "clockingOffset", "" ).toLatin1(),
                                         e.attribute( "torqueLimit", "2.5" ).toLatin1(),
                                         e.attribute( "timeLimit", "1" ).toLatin1(),
                                         e.attribute( "limitBlinkEnabled", "-1" ).toInt(),
//                                         e.attribute( "reverseAngle", "0" ).toLatin1(),
                                         e.attribute( "measurementVersion", "0" ).toInt(),
                                         e.attribute( "Speed1", "0" ).toLatin1(),
                                         e.attribute( "Speed2", "0" ).toLatin1(),
                                         e.attribute( "Speed3", "0" ).toLatin1(),
                                         e.attribute( "disclaimerText", InvestigationalDeviceWarning ),
                                         speedType,
                                         d1Img );    // d1Img

                deviceList.append( d1 );
            }
        }
        testno++;
        if (retVal != true) qDebug() << "test: " << testno;

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
    if(words.size() == 3){
        retVal = QString("%1 %2\n%3").arg(words[0]).arg(words[1]).arg(words[2]);
    }
    return retVal;
}

bool deviceSettings::getIsSimulation() const
{
    return m_isSimulation;
}

void deviceSettings::setIsSimulation(bool isSimulation)
{
    m_isSimulation = isSimulation;
}

device::device(QString inDeviceName, int inInternalImagingMask_px, int inCatheterRadius_um, int inLinesPerRevolution_cnt, int inRevolutionsPerMin, int inALineLengthNormal_px, int inALineLengthDeep_px, float inImagingDepthNormal_mm, float inImagingDepthDeep_mm, int inClockingEnabled, QByteArray inClockingGain, QByteArray inClockingOffset, QByteArray inTorqueLimit, QByteArray inTimeLimit, int inLimitBlinkEnabled, int inMeasurementVersion, QByteArray inSpeed1, QByteArray inSpeed2, QByteArray inSpeed3, QString inDisclaimerText, device::DeviceType inDeviceType, QImage *inIcon)
{
    deviceName               = inDeviceName;
    splitDeviceName          = formatDeviceName(deviceName);
    internalImagingMask_px   = inInternalImagingMask_px;
    catheterRadius_um        = inCatheterRadius_um;
    linesPerRevolution_cnt   = inLinesPerRevolution_cnt;
    revolutionsPerMin        = inRevolutionsPerMin;
    aLineLengthNormal_px     = inALineLengthNormal_px;
    aLineLengthDeep_px       = inALineLengthDeep_px;
    imagingDepthNormal_mm    = inImagingDepthNormal_mm;
    imagingDepthDeep_mm      = inImagingDepthDeep_mm;
    clockingEnabled          = inClockingEnabled;
    clockingGain             = inClockingGain;
    clockingOffset           = inClockingOffset;
    torqueLimit              = inTorqueLimit;
    timeLimit                = inTimeLimit;
    limitBlinkEnabled        = inLimitBlinkEnabled;
    //        reverseAngle             = inReverseAngle;

    disclaimerText           = inDisclaimerText;
    measurementVersion       = inMeasurementVersion;
    deviceType               = inDeviceType;
    icon                     = inIcon;
    pixelsPerMm              = (float)aLineLengthNormal_px / (float)imagingDepthNormal_mm;
    pixelsPerUm              = pixelsPerMm / (float)1000;
}

device::~device()
{
    if( icon != NULL )
    {
        delete icon;
    }
}

const QString &device::getDeviceName() const
{
    return deviceName;
}

const QString &device::getSplitDeviceName() const
{
    return splitDeviceName;
}
