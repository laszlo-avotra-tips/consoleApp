
#include <qt_windows.h>
#include <winnt.h>

#include <sysinfoapi.h>

#include "localTime.h"
#include "logger.h"

#include <QDebug>
#include <QDate>


LocalTime* LocalTime::m_instance{nullptr};

LocalTime *LocalTime::instance()
{
    if(!m_instance){
        m_instance = new LocalTime();
    }
    return m_instance;
}

bool LocalTime::addSeconds(int adjustmentInSeconds)
{
    bool success{false};
    if(adjustmentInSeconds){
        const int oneHourInSeconds = 60 *60;
        const int oneMinuteInSeconds = 60;

        const int deltaH = adjustmentInSeconds / oneHourInSeconds;
        const int deltaM = (adjustmentInSeconds % oneHourInSeconds) / oneMinuteInSeconds;
        const int deltaS = adjustmentInSeconds % oneMinuteInSeconds;

        LOG1(deltaH);
        LOG1(deltaM);
        LOG1(deltaS);

        _SYSTEMTIME current;
        GetLocalTime(&current);

        current.wHour += deltaH;
        current.wMinute += deltaM;
        current.wSecond += deltaS;

        success = updateLocalTime(current);
    }
    return success;
}

bool LocalTime::setDate(const QDate &date)
{
    bool success{false};
    bool isUpdateLocalTime{false};
    _SYSTEMTIME current{};
    GetLocalTime(&current);

    if(current.wYear != date.year()){
        current.wYear = date.year();
        isUpdateLocalTime = true;
    }

    if(current.wMonth != date.month()){
        current.wMonth = date.month();
        isUpdateLocalTime = true;
    }

    if(current.wDay != date.day()){
        current.wDay = date.day();
        isUpdateLocalTime = true;
    }

    if(current.wDayOfWeek != date.dayOfWeek()){
        current.wDayOfWeek = date.dayOfWeek();
        isUpdateLocalTime = true;
    }

    if(isUpdateLocalTime){
        success = updateLocalTime(current);
    }
    return success;
}

bool LocalTime::setLocalTime(const QTime &time)
{
    _SYSTEMTIME current{};
    GetLocalTime(&current);

    current.wHour = time.hour();
    current.wMinute = time.minute();
    current.wSecond = time.second();
    current.wMilliseconds = 0;

    return updateLocalTime(current);
}

bool LocalTime::updateLocalTime(const _SYSTEMTIME &st)
{
    bool success{true};
    success = SetLocalTime(&st);
    logSystemTime(st);
    LOG1(success);
    return success;
}

LocalTime::LocalTime()
{
}

void LocalTime::logSystemTime(const _SYSTEMTIME &current)
{
    LOG1(current.wYear);
    LOG1(current.wMonth);
    LOG1(current.wDayOfWeek);
    LOG1(current.wDay);

    LOG1(current.wHour);
    LOG1(current.wMinute);
    LOG1(current.wSecond);
}
