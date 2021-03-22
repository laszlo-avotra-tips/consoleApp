#ifndef LOCALTIME_H
#define LOCALTIME_H

class QDate;
class QTime;
struct _SYSTEMTIME;

class LocalTime
{
public:
    static LocalTime* instance();
    bool addSeconds(int adjustmentInSeconds);
    bool setDate(const QDate& date);
    bool setLocalTime(const QTime& time);
    bool updateLocalTime(const _SYSTEMTIME &st);

private:
    static LocalTime* m_instance;

    LocalTime();
    void logSystemTime(const _SYSTEMTIME &current);
};

#endif // LOCALTIME_H
