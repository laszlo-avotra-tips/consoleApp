/*
 * windowmanager.h
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
 * Copyright (c) 2017 Avinger, Inc.
 */
#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H
#include <QDebug>
#include <QDesktopWidget>
#include <QObject>
#include <QApplication>
#include <QRect>
#include "Windows.h"
#include "styledmessagebox.h"
#include <QTimer>

class WindowManager : public QObject
{
    Q_OBJECT

public:
    // Singleton
    static WindowManager &Instance()
    {
        static WindowManager theWinMgr;
        return theWinMgr;
    }

    void init();

    QRect getDefaultDisplayGeometry();
    QRect getTechnicianDisplayGeometry();
    QRect getPhysicianDisplayGeometry();
    QRect getAuxilliaryDisplayGeometry();
    bool  isPhysicianMonPresent();
    bool  isAuxMonPresent();
    QString rectToString( QRect rect );
    int numScreens();
    void showInfoMessage(QWidget *parent);
    void hideInfoMessage();
    void moveAllWindowsToPrimaryScreen();
    void centerChildOverParent( QWidget *child );

public slots:
    void handleWindowingSystemEvents();
    bool runFullTest();
    void configure();

signals:
    void monitorChangesDetected();
    void badMonitorConfigDetected();

private slots:
    void setScreenGeometries();
//    void configure();

private:
    void configureMonitors();
    void getFriendlyNameFromTarget( LUID adapterId, UINT32 targetId, int myMonitorEnum );
    bool enumerateMonitorNames();
    void getGDIDeviceNameFromSource( LUID adapterId, UINT32 sourceId );
    void getMonitorDevicePathFromTarget( LUID adapterId, UINT32 targetId );
    QWidgetList sortWidgetList( QWidgetList list );

    QDesktopWidget *qdw;
    QRect TechnicianDisplayRect;
    QRect PhysicianDisplayRect;
    QRect AuxilliaryDisplayRect;
    int   expectedTechEnum;
    int   failedTestCount;
    bool  goodMonitorConfigFound;
    QTimer *debounceTimer;
    styledMessageBox *infoBox;

    // prevent access to:
    WindowManager();     //   default constructor
    ~WindowManager();    //   default destructor
};

#endif // WINDOWMANAGER_H
