#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

class Logger
{
public:
    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    static void log(LogLevel level, const QString& message, const QString& userId = "");
    static void setupLogger();

private:
    static QString levelToString(LogLevel level);
};

#endif // LOGGER_H
