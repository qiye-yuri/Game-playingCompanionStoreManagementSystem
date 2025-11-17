#include "DatabaseManager.h"
#include <QMessageBox>
#include <QSqlError>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::openDatabase()
{
    if(m_db.isOpen()) return true;

    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName(m_hostName);
    m_db.setDatabaseName(m_databaseName);
    m_db.setUserName(m_userName);
    m_db.setPassword(m_password);
    m_db.setPort(m_port);

    if(!m_db.open()) {
        qCritical() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Database connected successfully";
    return true;
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

void DatabaseManager::closeDatabase()
{
    if(m_db.isOpen()) {
        m_db.close();
        qDebug() << "Database connection closed";
    }
}
