#ifndef PREFERENCESDATABASE_H
#define PREFERENCESDATABASE_H

#include <QtSql>
#include <set>

using TableContainers_t = std::set<QString>;

class PreferencesDatabase
{
public:
    PreferencesDatabase();
    ~PreferencesDatabase();

    int addPhysician(const QString& name);
    int addLocation(const QString& name);

    const TableContainers_t& physicians() const;
    const TableContainers_t& locations() const;

    void initCaseInfo();
    void initContainers();
    void updatePhysicianTable(const std::set<QString>& names);
    void updateLocationTable(const std::set<QString>& names);

private:
    QSqlError createTables();

    const QString m_dbName{"C:/Avinger_System/preferences.db"};
    QSqlDatabase m_db;

    TableContainers_t m_physicians;
    TableContainers_t m_locations;
};

#endif // PREFERENCESDATABASE_H

