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
