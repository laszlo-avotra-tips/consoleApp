/*
 * windowmanager.cpp
 *
 * Gather all desktop info so it is accessible to others.
 *
 * Provide method to get layout of each monitor/screen.
 * Provide signals to reconfiguring displays/monitors.
 * This class uses the singleton design pattern to prevent multiple instances
 * throughout the program.
 *
 * Author: Ryan F. Radjabi
 *
 * Copyright (c) 2017-2018 Avinger, Inc.
 */
#include "windowmanager.h"
#include "defaults.h"
#include <QMainWindow>

namespace {
const char* TechMonitorName = "PCT2265";
const int MaxTestIterations = 5;
}

/*
 * WARNING! - Do not turn this define off in the repository (for shipping releases).
 * The WindowManager code is VERY sensitive to timing issues on different hardware
 * and the many debug lines seem to be needed for some systems/monitors.
 */
#define MSG_ON 1

/*
 * Constructor
 */
WindowManager::WindowManager()
{
    infoBox = nullptr;
    goodMonitorConfigFound = false;
    expectedTechEnum = -1; // enumeration in the context of WinAPI starts at 1
    failedTestCount = 0;
    qdw = QApplication::desktop();

    // construct null QRect
    TechnicianDisplayRect = QRect();
    PhysicianDisplayRect  = QRect();
    AuxilliaryDisplayRect = QRect();

    connect( qdw, SIGNAL(screenCountChanged(int)), this, SLOT(handleWindowingSystemEvents()) );
    connect( qdw, SIGNAL(resized(int)), this, SLOT(handleWindowingSystemEvents()) );
    connect( qdw, SIGNAL(workAreaResized(int)), this, SLOT(handleWindowingSystemEvents()) );

    debounceTimer = new QTimer();
    debounceTimer->setSingleShot( true );
    connect( debounceTimer, SIGNAL(timeout()), this, SLOT(configure()) );
}

/* 
 * init
 */
void WindowManager::init()
{
#if QT_NO_DEBUG
    configure(); // force the first test and config
#endif
}

/*
 * Destructor
 */
WindowManager::~WindowManager()
{
}

/*
 * configureMonitors
 *
 * This routine will arrange the relative position of monitors as well as force the Technician monitor to be the Primary.
 * Code from: https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/95267766-8416-4f8a-be76-881ca8f32cc3/switching-primary-monitor-with-changedisplaysettingsex?forum=windowssdk
 */
void WindowManager::configureMonitors()
{
    // set szDeviceName to primary monitor
    DEVMODE deviceMode;
    deviceMode.dmSize = sizeof(DEVMODE);
    deviceMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_POSITION | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS;

    DISPLAY_DEVICE DisplayDevice;
    ZeroMemory(&DisplayDevice, sizeof(DISPLAY_DEVICE));
    DisplayDevice.cb = sizeof(DisplayDevice);

    int numDisplays = 0;
    for(int i = 0; EnumDisplayDevices(nullptr, i, &DisplayDevice, 0); i++)
    {
        if(!(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && (DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
        {
            numDisplays++;
        }
    }

    ZeroMemory(&DisplayDevice, sizeof(DISPLAY_DEVICE));
    DisplayDevice.cb = sizeof(DisplayDevice);

    for(int i = 0; EnumDisplayDevices(nullptr, i, &DisplayDevice, 0); i++)
    {
        if(!(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && (DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
        {
            EnumDisplaySettings(DisplayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode);

            QString newStr = QString::fromWCharArray(DisplayDevice.DeviceName);
#if MSG_ON
            qDebug() << "newStr: " << newStr;
#endif

            QString compStr;
            compStr = QString( "DISPLAY" + QString::number( expectedTechEnum ) );
#if MSG_ON
            qDebug() << "compStr: " << compStr;
#endif

            //test compare
            bool containsReturn = newStr.contains( compStr, Qt::CaseInsensitive );
#if MSG_ON
            qDebug() << "Contains returns: " << containsReturn;
#endif

            if( containsReturn )
            {
                deviceMode.dmPosition.x = 0;
                deviceMode.dmPosition.y = 0;
#if MSG_ON
                qDebug() << "ChangeDisplaySettingsEx returns: " <<
#endif
                            ChangeDisplaySettingsEx(reinterpret_cast<LPCWSTR>(DisplayDevice.DeviceName), &deviceMode, nullptr, CDS_SET_PRIMARY | CDS_UPDATEREGISTRY | CDS_NORESET, nullptr);

#if MSG_ON
                qDebug() << "DEVMODE dmDeviceName" << QString::fromWCharArray( deviceMode.dmDeviceName );
                qDebug() << "Display device: " << QString::fromWCharArray( DisplayDevice.DeviceName ) << "dev string: " << QString::fromWCharArray( DisplayDevice.DeviceString );
#endif
            }
            else
            {
                deviceMode.dmPosition.x = DM_PELSWIDTH;
                deviceMode.dmPosition.y = 0;
#if MSG_ON
                qDebug() << "ChangeDisplaySettingsEx returns: " << ChangeDisplaySettingsEx(reinterpret_cast<LPCWSTR>(DisplayDevice.DeviceName), &deviceMode, nullptr, CDS_UPDATEREGISTRY | CDS_NORESET, nullptr);
#endif
            }
#if MSG_ON
            qDebug() << "EnumDisplaySettings: " <<
#endif
                        reinterpret_cast<int>(EnumDisplaySettings(DisplayDevice.DeviceName, i, &deviceMode));
#if MSG_ON
            qDebug() << QString::fromWCharArray( DisplayDevice.DeviceName ) << "DisplayDevice.StateFlags DISPLAY_DEVICE_PRIMARY_DEVICE: "
                     << reinterpret_cast<unsigned long>((DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
                     << "Position:" << deviceMode.dmPosition.x << deviceMode.dmPosition.y;
#endif
        }
    }

#if MSG_ON
    qDebug() << "ChangeDisplaySettingsEx returns: " <<
#endif
                ChangeDisplaySettingsEx(nullptr,nullptr,nullptr,0,nullptr);

    ZeroMemory(&DisplayDevice, sizeof(DISPLAY_DEVICE));
    DisplayDevice.cb = sizeof(DisplayDevice);
    for(int i = 0; EnumDisplayDevices(nullptr, i, &DisplayDevice, 0); i++)
    {
        if(!(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && (DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
        {
            EnumDisplaySettings(DisplayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode);
#if MSG_ON
            qDebug() << "EnumDisplaySettings: " <<
#endif
                        reinterpret_cast<int>(EnumDisplaySettings(DisplayDevice.DeviceName, i, &deviceMode));
#if MSG_ON
            qDebug() << QString::fromWCharArray( DisplayDevice.DeviceName ) << "DisplayDevice.StateFlags DISPLAY_DEVICE_PRIMARY_DEVICE: "
                     << reinterpret_cast<unsigned long>((DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
                     << "Position:" << deviceMode.dmPosition.x << deviceMode.dmPosition.y;
#endif
        }
    }

#if MSG_ON
    for( int i = 0; i < qdw->screenCount(); i++ )
    {
        qDebug() << "screen geometry: " << qdw->screenGeometry( i ) << "isPrimary: " << ( qdw->primaryScreen() == i );
    }

    qDebug() << "primary screen: " << qdw->primaryScreen() << "geometry: " << qdw->screenGeometry( qdw->primaryScreen() );
#endif
}

/*
 * runFullTest
 *
 * This is the appropriate sequence for checking monitors and getting the appropriate display resolutions.
 */
bool WindowManager::runFullTest()
{
    bool retVal = false;
    if( enumerateMonitorNames() )
    {
        failedTestCount = 0;
        retVal = true;
    }
    else
    {
        failedTestCount++;
        retVal = false;
    }

    /*
     * It is necessary to call configureMonitors, setScreenGeometries, and monitorChangesDetected
     * even upon a FAILED enumerateMonitorNames call. This helps prevent displays being placed on
     * wrong monitors for some pesky combinations of Lightbox and monitors.
     */
    configureMonitors();
    setScreenGeometries();
    emit monitorChangesDetected();

    return retVal;
}

/*
 * handleWindowingSystemEvents
 *
 * This is a slot to QDesktopWidget's signals. Many signals may be emitted for the single connect
 * /disconnect event, in which case we want to debounce and ultimately call configure once. When
 * the timer expires, configure is called (singleshot).
 */
void WindowManager::handleWindowingSystemEvents()
{
    // debounce events
    debounceTimer->start( 3000 ); // 3 second debounce is a good place
}

/*
 * configure
 *
 * This should be called once for each sequence of monitor connect/disconnect. Tests to see
 * if runFullTest() has failed too many times, if so, it won't attempt any more configuration.
 */
void WindowManager::configure()
{
#if !( ENABLE_LAPTOP_MODE || ENABLE_DEMO_MODE )
    goodMonitorConfigFound = false;
    failedTestCount = 0; // clear the count

    while( failedTestCount < MaxTestIterations )
    {
        if( runFullTest() )
        {
            goodMonitorConfigFound = true;
            break; // break if a good config is found
        }
    }

    if( !goodMonitorConfigFound )
    {
        qDebug() << "bad monitor config detected..." << failedTestCount << MaxTestIterations;
        emit badMonitorConfigDetected();
        failedTestCount = 0;
    }
#else
    setScreenGeometries();
#endif
}

/*
 * enumerateMonitorNames
 *
 * Enumerates all monitors and checks the make/model name. This calls getFriendlyNameFromTarget() which then compares
 * with the expected make/model of the Technician monitor. If that monitor is found, it is marked as the primary monitor
 * and we are set for everything else.
 * Code from: https://social.msdn.microsoft.com/Forums/vstudio/en-US/cede9420-bc64-45e9-867c-f15920fb2c98/c-enumerate-monitor-name-get-same-name-used-in-control-panel-screen-resolution?forum=csharpgeneral
 */
bool WindowManager::enumerateMonitorNames()
{
    expectedTechEnum = -1;
    bool retVal = false;
//    UINT32 num_of_paths = 0;
//    UINT32 num_of_modes = 0;
//    DISPLAYCONFIG_PATH_INFO* displayPaths = nullptr;
//    DISPLAYCONFIG_MODE_INFO* displayModes = nullptr;

//    GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &num_of_paths, &num_of_modes);

//    // Allocate paths and modes dynamically
//    displayPaths = (DISPLAYCONFIG_PATH_INFO*)calloc((int)num_of_paths, sizeof(DISPLAYCONFIG_PATH_INFO));
//    displayModes = (DISPLAYCONFIG_MODE_INFO*)calloc((int)num_of_modes, sizeof(DISPLAYCONFIG_MODE_INFO));

//    // Query for the information
//    QueryDisplayConfig(QDC_ALL_PATHS, &num_of_paths, displayPaths, &num_of_modes, displayModes, nullptr);

//    int myMonitorEnum = 1;

//    for( unsigned int i = 0; i < num_of_modes; i++ )
//    {
//        switch (displayModes[i].infoType) {


//            // This case is for all sources
//            case DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE:
//                getGDIDeviceNameFromSource(displayModes[i].adapterId, displayModes[i].id);
//#if MSG_ON
//                printf("  Source AID: %d\r\n", displayModes[i].adapterId);
//                printf("  Source  ID: %d\r\n", displayModes[i].id);
//#endif
//                myMonitorEnum++;
//                break;

//            // This case is for all targets
//            case DISPLAYCONFIG_MODE_INFO_TYPE_TARGET:
//                getMonitorDevicePathFromTarget(displayModes[i].adapterId, displayModes[i].id);
//                getFriendlyNameFromTarget(displayModes[i].adapterId, displayModes[i].id, myMonitorEnum);
//#if MSG_ON
//                printf("  Target AID: %d\r\n", displayModes[i].adapterId);
//                printf("  Target  ID: %d\r\n", displayModes[i].id);
//#endif
//                break;

//            default:
//                fputs("error", stderr);
//                break;
//        }
//    }

//    retVal = ( expectedTechEnum > 0 );

//    free(displayPaths);
//    free(displayModes);

    return retVal;
}

/*
 * getFriendlyNameFromTarget
 *
 * (e.g. "PCT2265")
 */
void WindowManager::getFriendlyNameFromTarget( LUID adapterId, UINT32 targetId, int myMonitorEnum )
{
//    DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName;
//    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
//    header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
//    header.adapterId = adapterId;
//    header.id = targetId;
//    header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
//    deviceName.header = header;
//    DisplayConfigGetDeviceInfo( (DISPLAYCONFIG_DEVICE_INFO_HEADER*) &deviceName );
//#if MSG_ON
//    printf("  monitor friendly name: ");
//    qDebug() << "Monitor friendly name: " << QString::fromWCharArray(deviceName.monitorFriendlyDeviceName) << "myMonitorEnum: " << myMonitorEnum;

//    qDebug() << " String compare " << QString::fromWCharArray( deviceName.monitorFriendlyDeviceName ) << TechMonitorName
//             << "compare: " << QString::fromWCharArray( deviceName.monitorFriendlyDeviceName ).compare( TechMonitorName );
//#endif
//    if( QString::fromWCharArray( deviceName.monitorFriendlyDeviceName ).compare( TechMonitorName ) == 0 )
//    {
//        // Tech monitor found, set it to the monitor enum.
//        expectedTechEnum = myMonitorEnum;
//#if MSG_ON
//        qDebug() << "setting expectedTechEnum: " << myMonitorEnum;
//#endif
//    }
//    else
//    {
//#if MSG_ON
//        qDebug() << "failed to set expectedTechEnum.";
//#endif
//    }

//#if MSG_ON
//    wprintf(deviceName.monitorFriendlyDeviceName);
//    puts("");
//#endif
}

/*
 * getGDIDeviceNameFromSource
 *
 * (e.g. \\.\DISPLAY4)
 */
void WindowManager::getGDIDeviceNameFromSource( LUID adapterId, UINT32 sourceId )
{
//    DISPLAYCONFIG_SOURCE_DEVICE_NAME deviceName;
//    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
//    header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
//    header.adapterId = adapterId;
//    header.id = sourceId;
//    header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
//    deviceName.header = header;
//    DisplayConfigGetDeviceInfo( (DISPLAYCONFIG_DEVICE_INFO_HEADER*) &deviceName );
}

/*
 * getMonitorDevicePathFromTarget
 *
 * (e.g. \\?\DISPLAY#SAM0304#5&9a89472&0&UID33554704#{e6f07b5f-ee97-4a90-b076-33f57bf4eaa7})
 */
void WindowManager::getMonitorDevicePathFromTarget( LUID adapterId, UINT32 targetId )
{
//    DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName;
//    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
//    header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
//    header.adapterId = adapterId;
//    header.id = targetId;
//    header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
//    deviceName.header = header;
//    DisplayConfigGetDeviceInfo( (DISPLAYCONFIG_DEVICE_INFO_HEADER*) &deviceName );
}

/*
 * getTechnicianDisplayGeometry
 */
QRect WindowManager::getTechnicianDisplayGeometry()
{
    if( goodMonitorConfigFound )
    {
        return TechnicianDisplayRect;
    }
    else
    {
        return getDefaultDisplayGeometry();
    }
}

/*
 * getPhysicianDisplayGeometry
 */
QRect WindowManager::getPhysicianDisplayGeometry()
{
    return PhysicianDisplayRect;
}

/*
 * getAuxilliaryDisplayGeometry
 */
QRect WindowManager::getAuxilliaryDisplayGeometry()
{
    return AuxilliaryDisplayRect;
}

/*
 * isPhysicianMonPresent
 *
 * Accessor function to know if a Physician Monitor is detected.
 */
bool WindowManager::isPhysicianMonPresent()
{
    return ( !PhysicianDisplayRect.isNull() );
}

/*
 * isAuxMonPresent
 *
 * Accessor function to know if an Aux Monitor is detected.
 */
bool WindowManager::isAuxMonPresent()
{
    return ( !AuxilliaryDisplayRect.isNull() );
}

/*
 * getDefaultDisplayGeometry
 */
QRect WindowManager::getDefaultDisplayGeometry()
{
    return( qdw->screenGeometry( qdw->primaryScreen() ) );
}

/*
 * setScreenGeometries
 *
 * Based on the number of displays detected, determine the appropriate
 * resolution/layout geomtry for each monitor.
 */
void WindowManager::setScreenGeometries()
{
    for( int i = 0; i < qdw->screenCount(); i++ )
    {
#if !( ENABLE_LAPTOP_MODE || ENABLE_DEMO_MODE )
        if( qdw->screenGeometry( i ).width()  == TechScreenWidth &&
            qdw->screenGeometry( i ).height() == TechScreenHeight &&
            qdw->primaryScreen() == i )
#else
        if( qdw->primaryScreen() == i )
#endif
        {
            TechnicianDisplayRect = qdw->screenGeometry( i );
        }
        else if( qdw->screenGeometry( i ).width()  == PhysicianScreenWidth &&
                 qdw->screenGeometry( i ).height() == PhysicianScreenHeight &&
                 qdw->primaryScreen() != i )
        {
            PhysicianDisplayRect = qdw->screenGeometry( i );
        }
        else if( qdw->screenGeometry( i ).width()  >= AuxScreenWidthMin &&
                 qdw->screenGeometry( i ).height() >= AuxScreenHeightMin &&
                 qdw->primaryScreen() != i )
        {
            if( qdw->screenGeometry( i ) != AuxilliaryDisplayRect )
            {
                AuxilliaryDisplayRect = qdw->screenGeometry( i );
            }
        }
        else
        {
            AuxilliaryDisplayRect = QRect(); // if the resolution of aux monitor is too low
        }
    }

    if( qdw->screenCount() < 3 )
    {
        AuxilliaryDisplayRect = QRect(); // set to null
    }
}

/*
 * rectToString
 *
 * Helpful for debugging and logging QRect.
 */
QString WindowManager::rectToString( QRect rect )
{
    return QString( "%1, %2, %3, %4" ).arg( rect.left() ).arg( rect.top() ).arg( rect.right() ).arg( rect.bottom() );
}

/*
 * numScreens
 *
 * Return the number of screens connected.
 */
int WindowManager::numScreens()
{
    return qdw->screenCount();
}

/*
 * showInfoMessage
 */
void WindowManager::showInfoMessage( QWidget *parent )
{
    // hide any message that might previously exist
    hideInfoMessage();

    if( !infoBox )
    {
        infoBox = new styledMessageBox( parent );
        infoBox->setTitle( tr( "WARNING" ) );
        infoBox->setInstructions( tr( "Please connect the technician monitor." ) );
        infoBox->setText( QString( tr( "If problems persist, please contact Avinger Service at %1." ) ).arg( ServiceNumber ) );
        infoBox->setHasCancel( false, false, false );
        infoBox->exec();
    }
}

/*
 * hideInfoMessage
 */
void WindowManager::hideInfoMessage()
{
    if( infoBox )
    {
        infoBox->close();
        delete infoBox;
        infoBox = nullptr;
    }
}

/*
 * moveAllWindowsToPrimaryScreen
 *
 * Use the sorted list of widgets and move them.
 */
void WindowManager::moveAllWindowsToPrimaryScreen()
{
    QWidgetList list = sortWidgetList( QApplication::allWidgets() );

    foreach( QWidget *w, list )
    {
        centerChildOverParent( w );
    }
}

/*
 * centerChildOverParent
 *
 * Move the child widget over the parent. If no parent exists, use a default widget for reference location.
 */
void WindowManager::centerChildOverParent( QWidget *child )
{
    bool isFullScreen = child->isFullScreen();
    QWidget *parent;

    if( !child->parent() )
    {
        parent = QApplication::widgetAt(0,0);
        if( !parent )
        {
            return;
        }
    }
    else
    {
        parent = child->parentWidget();
    }


    int newX, newY;
    newX = parent->geometry().center().x() - ( child->width() / 2 );
    newY = parent->geometry().center().y() - ( child->height() / 2 );

    child->setGeometry( newX,               // newX
                        newY,               // newY
                        child->width(),     // orig w
                        child->height() );  // orig h

    if( isFullScreen )
    {
        child->showFullScreen();
    }
}

/*
 * sortWidgetList
 *
 * Filter out widgets we won't try to move and order the widgets with parents first and children of parents
 * and orphans last.
 */
QWidgetList WindowManager::sortWidgetList( QWidgetList list )
{
    QWidgetList retList;

    for( int i = 0; i < list.size(); i++ )
    {
        bool isQualified = false;
        if( list.at( i )->isWidgetType() ) // is Widget
        {
            if( list.at( i )->isWindow() ) // is Window
            {
                if( !qobject_cast<QMainWindow*>( list.at( i ) ) ) // is not MainWindow
                {
                    // proceded
                    isQualified = true;
                }
            }
        }

        if( isQualified )
        {
            // re-order based on criteria, 1) hasParent && hasChild(ren) 2) hasParent
            // this is the first pass, so move to front if hasParent
            if( list.at( i )->parent() )
            {
                retList.prepend( list.at( i ) );
            }
            else
            {
                retList.append( list.at( i ) );
            }
        }
    }

    QWidgetList finalRetList;

    for( int i = 0; i < retList.size(); i++ )
    {
        bool sendToFront = false;
        // all should be qualified now, just re-order based on criteria 1) hasParent && hasChild(ren)
        if( retList.at( i )->parent() )
        {
            if( !retList.at( i )->children().isEmpty() )
            {
                // test if has children that are windows
                foreach( QObject *obj, retList.at( i )->children() )
                {
                    QWidget *w = qobject_cast<QWidget*>(obj);
                    if( w )
                    {
                        if( w->isWindow() )
                        {
                            sendToFront = true;
                            break;
                        }
                    }
                }
            }
        }

        if( sendToFront )
        {
            finalRetList.prepend( retList.at( i ) );
        }
        else
        {
            finalRetList.append( retList.at( i ) );
        }
    }

    return finalRetList;
}
