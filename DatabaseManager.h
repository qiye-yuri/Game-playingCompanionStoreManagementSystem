#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    bool openDatabase();
    void closeDatabase();
    bool isOpen() const { return m_db.isOpen(); }

private:
    DatabaseManager() = default;
    ~DatabaseManager();

    QSqlDatabase m_db;
    const QString m_hostName = "localhost";
    const QString m_databaseName = "x_manage_system";
    const QString m_userName = "root";
    const QString m_password = "83823833Aa";
    const int m_port = 3306;
};

#endif // DATABASEMANAGER_H
