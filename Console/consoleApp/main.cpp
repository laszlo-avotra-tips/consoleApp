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
#include "screenNavigator.h"
#include <QApplication>
#include <Backend/powerUpDiagnostics.h>
#include "logger.h"
#include <styledmessagebox.h>

void hookupPowerUpDiagnostics() {
    auto diagnostics = new PowerUpDiagnostics();
    auto messageBox = styledMessageBox::instance(); //new PowerUpMessageBox();
    LOG(INFO, "Initializing power up diagnostics");

    QObject::connect(diagnostics, &OctSystemDiagnostics::showMessageBox,
                     messageBox, &styledMessageBox::onShowMessageBox);
    QObject::connect(diagnostics, &OctSystemDiagnostics::hideMessageBox,
                     messageBox, &styledMessageBox::onHideMessageBox);

    QObject::connect(messageBox, &styledMessageBox::userAcknowledged,
                     diagnostics, &OctSystemDiagnostics::onUserAcknowledged);
    diagnostics->performDiagnostics(true);
}

/*
 * main
 */
int main(int argc, char *argv[])
{
    QApplication app( argc, argv );
    hookupPowerUpDiagnostics();

    ScreenNavigator navigator;
    navigator.display();

    app.exec();

    return 0;
}
