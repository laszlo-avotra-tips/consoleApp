/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info.com)
**
** This file is part of a Qt Solutions component.
**
** Commercial Usage
** Licensees holding valid Qt Solutions licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales.com.
**
****************************************************************************/

#include "qtsingleapplication.h"
#include <qt_windows.h>
#include <QWidget>
#include <logger.h>

class QtSingletonSysPrivate : public QWidget
{
public:
    QtSingletonSysPrivate()
        : QWidget(), listening(false)
    {
    }
    uint listening :1;

protected:
    bool winEvent(MSG *msg, long *result)
    {
        if (msg->message != WM_COPYDATA)
            return false;

//lcv        Q_ASSERT(msg->hwnd == winId());
        COPYDATASTRUCT *data = reinterpret_cast<COPYDATASTRUCT*>(msg->lParam);

        QString message = QString::fromUtf16( static_cast<ushort*>(data->lpData) );

        auto* app = dynamic_cast<QtSingleApplication*>(qApp);

        if(app){
            emit (app->messageReceived( message ) );
        }

	if (result)
	  *result = 0;
        return true;
    }

};

static HANDLE createLockedMutex(const QString &id)
{
    LOG1(id)
    HANDLE mutex = nullptr;
//    QT_WA({
//        mutex = CreateMutex(0, false, (TCHAR*)id.utf16());
//    }, {
//       mutex = CreateMutexA(0, false, id.toLocal8Bit().data());
//    });

//    switch (WaitForSingleObject(mutex, INFINITE)) {
//    case WAIT_ABANDONED:
//    case WAIT_TIMEOUT:
//        CloseHandle(mutex);
//        mutex = 0;
//        break;
//    default: // WAIT_OBJECT_0
//        break;
//    }

    return mutex;
}

static inline void closeLockedMutex(HANDLE handle)
{
    if (!handle)
        return;

    ReleaseMutex(handle);
    CloseHandle(handle);
}

static HWND findWindow(const QString &id)
{
    HANDLE mutex = createLockedMutex(id);

    HWND hwnd = nullptr;
//    QString wid = id + "_QtSingleApplicationWindow";

//    QT_WA( {
//	hwnd = ::FindWindow(L"QWidget", (TCHAR*)wid.utf16());
//    }, {
//	hwnd = ::FindWindowA("QWidget", wid.toLocal8Bit().data());
//    } )

    closeLockedMutex(mutex);

    return hwnd;
}

void QtSingleApplication::sysInit()
{
    sysd = new QtSingletonSysPrivate;
    (void)sysd->winId();                  // Force widget creation
}

void QtSingleApplication::sysCleanup()
{
    HANDLE mutex = createLockedMutex(id());

    delete sysd;

    closeLockedMutex(mutex);
}

void QtSingleApplication::initialize( bool activate )
{
    if (sysd->listening)
	return;

    HANDLE mutex = createLockedMutex(id());

    sysd->listening = true;
    sysd->setWindowTitle(id() + "_QtSingleApplicationWindow");

    closeLockedMutex(mutex);

    if ( activate )
	connect(this, SIGNAL(messageReceived(const QString&)),
		this, SLOT(activateWindow()));
}

bool QtSingleApplication::isRunning() const
{
    return findWindow(id()) != nullptr;
}

bool QtSingleApplication::sendMessage( const QString & message, int timeout )
{
    LOG2(message,timeout)
//    HWND hwnd = findWindow(id());
//    if ( !hwnd )
//    return false;

//    COPYDATASTRUCT data;
//    data.dwData = 0;
//    data.cbData = (message.length()+1) * sizeof(QChar);
//    data.lpData = (void*)message.utf16();
//    DWORD result(0);
//    LRESULT res = 0;
//    QT_WA( {
//	res = SendMessageTimeout(hwnd, WM_COPYDATA, 0/*hwnd sender*/, (LPARAM)&data,
//				 SMTO_ABORTIFHUNG,timeout,&result);
//    }, {
//	res = SendMessageTimeoutA(hwnd, WM_COPYDATA, 0/*hwnd sender*/, (LPARAM)&data,
//				  SMTO_ABORTIFHUNG,timeout,&result);
//    } )
//    return res != 0;
    return true;
}
