#ifndef DATETIMEMODEL_H
#define DATETIMEMODEL_H

#include <QObject>
#include <QDate>
#include <QTimer>


using AdjustmentContainer_t = std::map<QString, int>;

class DateTimeModel : public QObject
{
    Q_OBJECT
public:
    explicit DateTimeModel(QObject *parent = nullptr);

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
    void decrementMinutes();
    void decrementSeconds();
    bool isEditMode() const;

    int secondsAdjustment() const;
    QDate editDate() const;
    QTime editTime() const;

    void setEditTime(const QTime &editTime);

public slots:
    void applyTime();
    void applyDate();

signals:

private:
    AdjustmentContainer_t m_dateAdjustment{{"year",0},{"mon",0},{"day",0}};
    int m_secondsAdjustment{0};
    QDate m_editDate;
    QTime m_editTime;

    const int m_dayInSeconds{24*60*60};
};

#endif // DATETIMEMODEL_H
