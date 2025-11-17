#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QSettings>
#include <QString>

class ConfigManager
{
public:
    static ConfigManager& instance();

    QString databaseHost() const;
    QString databaseName() const;
    QString databaseUser() const;
    QString databasePassword() const;
    int databasePort() const;

private:
    ConfigManager();
    QSettings m_settings;
};

#endif // CONFIGMANAGER_H
