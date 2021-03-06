#include "dateTimeController.h"
#include "localTime.h"
#include "dateTimeModel.h"
#include "logger.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>


DateTimeController::DateTimeController(QWidget *parent)
    : QDialog(parent)
{
    m_model = new DateTimeModel(this);
}

void DateTimeController::controllerInitialize()
{
    m_lineEditDate->setInputMask("99/99/9999");
    m_lineEditDate->setReadOnly(true);

    m_lineEditTime->setInputMask("99:99:99");
    m_lineEditTime->setReadOnly(true);

    reloadDateTime();

    showCurrentDate();
    showCurrentTime();

    connect(m_lineEditDate, &QLineEdit::textChanged, this, &DateTimeController::handleDateChanged);
    connect(m_lineEditDate, &QLineEdit::cursorPositionChanged, this, &DateTimeController::handleDateCursorPositionChanged);

    connect(m_pushButtonDateDown, &QPushButton::clicked, this, &DateTimeController::handlePushButtonDateDown);
    connect(m_pushButtonDateUp, &QPushButton::clicked, this, &DateTimeController::handlePushButtonDateUp);

    connect(m_lineEditTime, &QLineEdit::cursorPositionChanged, this, &DateTimeController::handleTimeCursorPositionChanged);

    connect(m_pushButtonTimeDown, &QPushButton::clicked, this, &DateTimeController::handlePushButtonTimeDown);
    connect(m_pushButtonTimeUp, &QPushButton::clicked, this, &DateTimeController::handlePushButtonTimeUp);

    connect(m_pushButtonApply, &QPushButton::clicked, this, &DateTimeController::apply);
    connect(m_pushButtonCancel, &QPushButton::clicked, this, &DateTimeController::handlePushButtonCancel);

    connect(&m_updateTimer, &QTimer::timeout, this, &DateTimeController::showCurrentTime);

    m_updateTimer.start(m_updateTimerTimeout);
    m_pushButtonApply->setStyleSheet("background-color: black; color: black ;font: 16pt Helvetica Neue;");

    m_pushButtonCancel->setStyleSheet("background-color: black; color: black;font: 16pt Helvetica Neue; ");
}

void DateTimeController::cancelEdit()
{
    emit m_pushButtonCancel->clicked();
}

DateTimeController::~DateTimeController()
{
    m_updateTimer.stop();
}

void DateTimeController::setTimeWidgets(QLineEdit *le, QPushButton *up, QPushButton *down)
{
    m_lineEditTime = le;
    m_pushButtonTimeUp = up;
    m_pushButtonTimeDown = down;
}

void DateTimeController::setDateWidgets(QLineEdit *le, QPushButton *up, QPushButton *down)
{
    m_lineEditDate = le;
    m_pushButtonDateUp = up;
    m_pushButtonDateDown = down;
}

void DateTimeController::setWidgets(QPushButton *accept, QPushButton *cancel)
{
    m_pushButtonApply = accept;
    m_pushButtonCancel = cancel;
}

void DateTimeController::handlePushButtonDateUp()
{
    LOGUA;
    emit incrementDate();
}

void DateTimeController::handlePushButtonDateDown()
{
    LOGUA;
    emit decrementDate();
}

void DateTimeController::handlePushButtonTimeUp()
{
    LOGUA;
    emit incrementTime();
}

void DateTimeController::handlePushButtonTimeDown()
{
    LOGUA;
    emit decrementTime();
}

void DateTimeController::handlePushButtonCancel()
{
    LOGUA;
    if(isTimeEditMode()){
        setIsTimeEditMode(false);
        deselect();
    }
    if(isDateEditMode()){
        setIsDateEditMode(false);
        showDate(m_currentDate);
        m_model->setEditDate(m_currentDate);
        deselect();
    }

    showEditControlButtons(false);
}

void DateTimeController::incrementDay()
{
    LOGUA;
    m_model->incrementDay();
    showEditDate();
}

void DateTimeController::incrementMonth()
{
    LOGUA;
    m_model->incrementMonth();
    showEditDate();
}

void DateTimeController::incrementYear()
{
    m_model->incrementYear();
    showEditDate();
}

void DateTimeController::decrementDay()
{
    m_model->decrementDay();
    showEditDate();
}

void DateTimeController::decrementMonth()
{
    m_model->decrementMonth();
    showEditDate();
}

void DateTimeController::decrementYear()
{
    m_model->decrementYear();
    showEditDate();
}

void DateTimeController::incrementHour()
{
    m_model->incrementHour();
    showEditTime();
}

void DateTimeController::incrementMinutes()
{
    m_model->incrementMinutes();
    showEditTime();
}

void DateTimeController::incrementSeconds()
{
    m_model->incrementSeconds();
    showEditTime();
}

void DateTimeController::decrementHour()
{
    m_model->decrementHour();
    showEditTime();
}

void DateTimeController::decrementMimutes()
{
    m_model->decrementMinutes();
    showEditTime();
}

void DateTimeController::decrementSeconds()
{
    m_model->decrementSeconds();
    showEditTime();
}

void DateTimeController::handleDateChanged(const QString &)
{
    LOGUA;
    if(m_dateSelected.first != m_dateSelected.second){
        m_lineEditDate->setSelection(m_dateSelected.first, m_dateSelected.second);
    }
}

void DateTimeController::handleDateCursorPositionChanged(int, int pos)
{
    LOGUA;
    selectDateItem(pos);
}

void DateTimeController::handleTimeCursorPositionChanged(int, int pos)
{
    LOGUA;
    selectTimeItem(pos);
}

void DateTimeController::selectDateItem(int pos)
{
    setIsDateEditMode(true);

    if(isDateEditMode()){
        m_pushButtonDateDown->setEnabled(true);
        m_pushButtonDateUp->setEnabled(true);
    }

    //03/11/2021
    //0,1 month
    //3,4 day
    //6,7,8,9 year
    LOG1(pos);
    switch(pos){
    case 7:
    case 8:
    case 9:
    case 10:
        m_dateSelected = {6,4};
        m_lineEditDate->setSelection(m_dateSelected.first, m_dateSelected.second);
        disconnect();
        connect(this, &DateTimeController::incrementDate, this, &DateTimeController::incrementYear);
        connect(this, &DateTimeController::decrementDate, this, &DateTimeController::decrementYear);
        break;

    case 2:
    case 1:
    case 0:
        m_dateSelected = {0,2};
        m_lineEditDate->setSelection(m_dateSelected.first, m_dateSelected.second);
        disconnect();
        connect(this, &DateTimeController::incrementDate, this, &DateTimeController::incrementMonth);
        connect(this, &DateTimeController::decrementDate, this, &DateTimeController::decrementMonth);
        break;

    case 6:
    case 5:
    case 4:
    case 3:
        m_dateSelected = {3,2};
        m_lineEditDate->setSelection(m_dateSelected.first, m_dateSelected.second);
        disconnect();
        connect(this, &DateTimeController::incrementDate, this, &DateTimeController::incrementDay);
        connect(this, &DateTimeController::decrementDate, this, &DateTimeController::decrementDay);
        break;
    }
}

void DateTimeController::selectTimeItem(int pos)
{
    if(isTimeEditMode()){
        m_pushButtonTimeDown->setEnabled(true);
        m_pushButtonTimeUp->setEnabled(true);
    }

    //01:24:39
    //0,1 hour
    //3,4 min
    //6,7 sec
    LOG1(pos);
    switch(pos){
    case 2:
    case 1:
    case 0:
        setIsTimeEditMode(true);
        m_timeSelected = {0,2};
        m_lineEditTime->setSelection(m_timeSelected.first, m_timeSelected.second);
        disconnect();
        connect(this, &DateTimeController::incrementTime, this, &DateTimeController::incrementHour);
        connect(this, &DateTimeController::decrementTime, this, &DateTimeController::decrementHour);
        break;

    case 5:
    case 4:
    case 3:
        setIsTimeEditMode(true);
        m_timeSelected = {3,2};
        m_lineEditTime->setSelection(m_timeSelected.first, m_timeSelected.second);
        disconnect();
        connect(this, &DateTimeController::incrementTime, this, &DateTimeController::incrementMinutes);
        connect(this, &DateTimeController::decrementTime, this, &DateTimeController::decrementMimutes);
        break;

//    case 8:
    case 7:
    case 6:
        setIsTimeEditMode(true);
        m_timeSelected = {6,2};
        m_lineEditTime->setSelection(m_timeSelected.first, m_timeSelected.second);
        disconnect();
        connect(this, &DateTimeController::incrementTime, this, &DateTimeController::incrementSeconds);
        connect(this, &DateTimeController::decrementTime, this, &DateTimeController::decrementSeconds);
        break;
    }
}

void DateTimeController::showCurrentDate()
{
    if(!isDateEditMode()){
        showDate(m_currentDate);
    }
}

void DateTimeController::showEditDate()
{
    if(isDateEditMode()){
        showDate(m_model->editDate());
    }
}

void DateTimeController::showButton(bool isShown, QPushButton *button)
{
    LOG2(button->x(), button->y());
    if(isShown){
        //background-color:#676767;
        if(button->text() == "CANCEL"){
            button->setStyleSheet("background-color: #676767");
        }else{
            button->setStyleSheet("background-color: #F5C400");
        }
    }else{
        button->setStyleSheet("background-color: black; color: black ");
    }

//    if(isShown){
//        QPropertyAnimation *animation = new QPropertyAnimation(button, "geometry");
//        animation->setDuration(2000);

////        animation->setStartValue(QRect(button->x() - button->width() / 2, button->y() - button->height() / 2, 0,0));
//        animation->setStartValue(QRect(button->x(), button->y() - button->height() / 2, button->width(),0));
//        animation->setEndValue(QRect(button->x(), button->y() ,  button->width(), button->height()));
//        animation->setEasingCurve(QEasingCurve::OutBounce);

//        animation->start();
//    }

}

bool DateTimeController::isDateEditMode() const
{
    return m_isDateEditMode;
}

void DateTimeController::setIsDateEditMode(bool isDateEditMode)
{
    m_isDateEditMode = isDateEditMode;
    if(m_isDateEditMode){
        LOG1(m_isDateEditMode);
//        m_model->setEditDate(m_currentDate);
    }
    showEditControlButtons(m_isDateEditMode || m_isTimeEditMode);
}

bool DateTimeController::isTimeEditMode() const
{
    return m_isTimeEditMode;
}

void DateTimeController::setIsTimeEditMode(bool isTimeEditMode)
{
    if(m_isTimeEditMode != isTimeEditMode){
        m_isTimeEditMode = isTimeEditMode;
        if(m_isTimeEditMode){
            LOG1(m_isTimeEditMode);
            m_updateTimer.stop();
            m_model->setEditTime(m_currentTime);
        }else{
            m_updateTimer.start(m_updateTimerTimeout);
        }
    }
    showEditControlButtons(m_isDateEditMode || m_isTimeEditMode);
}

void DateTimeController::showEditControlButtons(bool isShown)
{
    showButton(isShown, m_pushButtonApply);
    showButton(isShown, m_pushButtonCancel);
    m_pushButtonDateDown->setEnabled(isShown);
    m_pushButtonDateUp->setEnabled(isShown);
}

void DateTimeController::showTime(const QTime& newTimeValue)
{
    QString newTime =  newTimeValue.toString("hh:mm:ss");

    m_lineEditTime->setText(newTime);
    if(m_timeSelected.first != m_timeSelected.second){
        m_lineEditTime->setSelection(m_timeSelected.first, m_timeSelected.second);
    }
}

void DateTimeController::showCurrentTime()
{
    if(!isTimeEditMode()){
        m_currentTime = QDateTime::currentDateTime().time();
        showTime(m_currentTime);
    }
}

void DateTimeController::showEditTime()
{
    if(isTimeEditMode()){
        showTime(m_model->editTime());
    }
}

void DateTimeController::apply()
{
    LOGUA;
    if(isTimeEditMode())
    {
        m_model->applyTime();
        setIsTimeEditMode(false);
        deselect();
    }
    if(isDateEditMode())
    {
        setIsDateEditMode(false);
        m_model->applyDate();
        deselect();
    }
    reloadDateTime();
}

void DateTimeController::deselect()
{
    LOGUA;
    m_dateSelected = {-1,-1};
    m_timeSelected = {-1,-1};
    m_lineEditDate->deselect();
    m_lineEditTime->deselect();
}

void DateTimeController::reloadDateTime()
{
    m_currentDate = QDateTime::currentDateTime().date();
    m_currentTime = QDateTime::currentDateTime().time();
    m_model->setEditDate(m_currentDate);
}

void DateTimeController::showDate(const QDate &newDate)
{
    m_lineEditDate->setText(newDate.toString("MM/dd/yyyy"));
    if(m_dateSelected.first != m_dateSelected.second){
        m_lineEditDate->setSelection(m_dateSelected.first, m_dateSelected.second);
    }
}
