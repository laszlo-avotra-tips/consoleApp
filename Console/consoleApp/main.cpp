/*
 * main.cpp
 *
 * The OCT Console application
 *
 * Author: Chris White, Dennis W. Jackson, Ryan Radjabi
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include <QObject>
#include <QCommandLineOption>
#include "qtsingleapplication.h"
#include "Screens/frontend.h"
#include "buildflags.h"
#include "initialization.h"
#include "laser.h"
#include "logger.h"
#include "util.h"
#include "version.h"
#include "Utility/sessiondatabase.h"
#include "powerdistributionboard.h"
#include "styledmessagebox.h"
#include "util.h"
#include "keyboardinputcontext.h"
#include "sledsupport.h"
#include "daqfactory.h"
#include "signalmanager.h"
#include "deviceSettings.h"
#include "backend.h"
#include "screenNavigator.h"
#include "keyboardinputcontext.h"


/*
 * main
 */
int main(int argc, char *argv[])
{
    QApplication app( argc, argv );

    ScreenNavigator navigator;
    navigator.display();

    app.exec();

    return 0;
}
