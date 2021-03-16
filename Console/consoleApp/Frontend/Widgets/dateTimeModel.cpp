#include "dateTimeModel.h"
#include "localTime.h"
#include "logger.h"

#include <QDebug>
#include <QDateTime>

//#define LOG2(x_,y_) {qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss:zzz")<< " "\
//    << __FUNCTION__ << ":"<< __LINE__ << " "<< #x_ << "=" << x_ << " "<< #y_ << "=" << y_ ;}
//#define LOG(x_) {qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss:zzz")<< " " << __FUNCTION__ << ":"<< __LINE__ << " "<< #x_ << "=" << x_;}
//#define LOG_ {qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss:zzz")<< " " << __FUNCTION__ << ":"<< __LINE__;}

DateTimeModel::DateTimeModel(QObject *parent) : QObject(parent)
{
    m_editDate = QDateTime::currentDateTime().date();
    m_editTime = QDateTime::currentDateTime().time();
}

void DateTimeModel::apply()
{

    applyDate();
    bool successTime = LocalTime::instance()->setLocalTime(m_editTime);
    LOG1(successTime);
}

void DateTimeModel::applyDate()
{
    bool successDate = LocalTime::instance()->setDate(m_editDate);
    LOG1(successDate);
    \
}

void DateTimeModel::setEditTime(const QTime &editTime)
{
    m_editTime = editTime;
}

void DateTimeModel::incrementDay()
{
    m_editDate = m_editDate.addDays(1);
    auto dayIt = m_dateAdjustment.find("day");
    if(dayIt != m_dateAdjustment.end()){
        ++dayIt->second;
        qDebug() << dayIt->second;
    }
}

void DateTimeModel::incrementMonth()
{
    m_editDate = m_editDate.addMonths(1);
    auto monthIt = m_dateAdjustment.find("mon");
    if(monthIt != m_dateAdjustment.end()){
        ++monthIt->second;
        qDebug() << monthIt->second;
    }
}

void DateTimeModel::incrementYear()
{
    m_editDate = m_editDate.addYears(1);
    auto yearIt = m_dateAdjustment.find("year");
    if(yearIt != m_dateAdjustment.end()){
        ++yearIt->second;
        qDebug() << yearIt->second;
    }
}

void DateTimeModel::decrementDay()
{
    m_editDate = m_editDate.addDays(-1);
    auto dayIt = m_dateAdjustment.find("day");
    if(dayIt != m_dateAdjustment.end()){
        --dayIt->second;
        qDebug() << dayIt->second;
    }
}

void DateTimeModel::decrementMonth()
{
    m_editDate = m_editDate.addMonths(-1);
    auto monthIt = m_dateAdjustment.find("mon");
    if(monthIt != m_dateAdjustment.end()){
        --monthIt->second;
        qDebug() << monthIt->second;
    }
}

void DateTimeModel::decrementYear()
{
    m_editDate = m_editDate.addYears(-1);
    auto yearIt = m_dateAdjustment.find("year");
    if(yearIt != m_dateAdjustment.end()){
        --yearIt->second;
        qDebug() << yearIt->second;
    }
}

void DateTimeModel::incrementHour()
{
    m_secondsAdjustment = 60 * 60;
    m_secondsAdjustment %= m_dayInSeconds;
    LOG1(m_secondsAdjustment);
    m_editTime = m_editTime.addSecs(m_secondsAdjustment);
}

void DateTimeModel::incrementMinutes()
{
    m_secondsAdjustment = 60;
    m_secondsAdjustment %= m_dayInSeconds;
    LOG1(m_secondsAdjustment);
    m_editTime = m_editTime.addSecs(m_secondsAdjustment);
}

void DateTimeModel::incrementSeconds()
{
    m_secondsAdjustment = 1;
    m_secondsAdjustment %= m_dayInSeconds;
    LOG1(m_secondsAdjustment);
    m_editTime = m_editTime.addSecs(m_secondsAdjustment);
}

void DateTimeModel::decrementHour()
{
    m_secondsAdjustment = -60 * 60;
    m_secondsAdjustment %= m_dayInSeconds;
    LOG1(m_secondsAdjustment);
    m_editTime = m_editTime.addSecs(m_secondsAdjustment);
}

void DateTimeModel::decrementMinutes()
{
    m_secondsAdjustment = -60;
    m_secondsAdjustment %= m_dayInSeconds;
    LOG1(m_secondsAdjustment);
    m_editTime = m_editTime.addSecs(m_secondsAdjustment);
}

void DateTimeModel::decrementSeconds()
{
    m_secondsAdjustment = -1;
    m_secondsAdjustment %= m_dayInSeconds;
    LOG1(m_secondsAdjustment);
    m_editTime = m_editTime.addSecs(m_secondsAdjustment);
}

int DateTimeModel::secondsAdjustment() const
{
    return m_secondsAdjustment;
}

QDate DateTimeModel::editDate() const
{
    return m_editDate;
}

QTime DateTimeModel::editTime() const
{
    LOG1(m_editTime.second());
    return m_editTime;
}
