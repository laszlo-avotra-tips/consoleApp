#ifndef DATETIMECONTROLLER_H
#define DATETIMECONTROLLER_H

#include <map>

#include <QDialog>
#include <QDate>
#include <QTimer>


class DateTimeModel;
class QLineEdit;
class QPushButton;

using AdjustmentContainer_t = std::map<QString, int>;

class DateTimeController : public QDialog
{
    Q_OBJECT

public:
    DateTimeController(QWidget *parent = nullptr);
    ~DateTimeController();

    void setTimeWidgets(QLineEdit* le, QPushButton* up, QPushButton* down);
    void setDateWidgets(QLineEdit* le, QPushButton* up, QPushButton* down);
    void setWidgets(QPushButton* accept, QPushButton* cancel);
    void controllerInitialize();
    void cancelEdit();
    void apply();

signals:
    void incrementDate();
    void decrementDate();

    void incrementTime();
    void decrementTime();

private slots:

    void handlePushButtonDateUp();
    void handlePushButtonDateDown();
    void handlePushButtonTimeUp();
    void handlePushButtonTimeDown();
    void handlePushButtonCancel();

    void handleDateCursorPositionChanged(int, int pos);
    void handleDateChanged(const QString &);

    void handleTimeCursorPositionChanged(int, int pos);
    void handleTimeChanged(const QString &);

    void incrementDay();
    void incrementMonth();
    void incrementYear();

    void decrementDay();
    void decrementMonth();
    void decrementYear();

    void incrementHour();
    void incrementMinutes();
    void incrementSeconds();

    void decrementHour();
    void decrementMimutes();
    void decrementSeconds();

    void showCurrentTime();
    void showEditTime();
    void showCurrentDate();
    void showEditDate();

    void deselect();
    void reloadDateTime();


private:
    void showDate(const QDate &newDate);
    void showTime(const QTime &newTimeValue);

    bool isTimeEditMode() const;
    void setIsTimeEditMode(bool isEditMode);
    bool isDateEditMode() const;
    void setIsDateEditMode(bool isDateEditMode);

    void showEditControlButtons(bool isShown);

    void selectDateItem(int pos);
    void selectTimeItem(int pos);

    void showButton(bool isShown, QPushButton* button);

    DateTimeModel* m_model{nullptr};

    QDate m_currentDate;
    QTime m_currentTime;

    std::pair<int,int> m_dateSelected{-1,-1};
    std::pair<int,int> m_timeSelected{-1,-1};

    QTimer m_updateTimer;
    const int m_updateTimerTimeout{100};

    bool m_isTimeEditMode{false};
    bool m_isDateEditMode{false};

    QLineEdit* m_lineEditDate{nullptr};
    QLineEdit* m_lineEditTime{nullptr};

    QPushButton* m_pushButtonDateDown{nullptr};
    QPushButton* m_pushButtonDateUp{nullptr};

    QPushButton* m_pushButtonTimeDown{nullptr};
    QPushButton* m_pushButtonTimeUp{nullptr};

    QPushButton* m_pushButtonApply{nullptr};
    QPushButton* m_pushButtonCancel{nullptr};

};

#endif // DATETIMECONTROLLER_H
