#include "Logger.h"
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDir>

void Logger::log(LogLevel level, const QString& message, const QString& userId)
{
    QString levelStr = levelToString(level);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logMessage;

    if (!userId.isEmpty()) {
        logMessage = QString("[%1] [%2] [用户:%3] %4").arg(timestamp, levelStr, userId, message);
    } else {
        logMessage = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
    }

    // 输出到控制台
    qDebug() << logMessage;

    // 输出到文件
    QFile file("logs/app.log");
    QDir().mkpath("logs"); // 确保logs目录存在

    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << logMessage << "\n";
        file.close();
    }
}

void Logger::setupLogger()
{
    // 创建日志目录
    QDir().mkpath("logs");

    // 设置日志文件最大大小和备份
    QFile file("logs/app.log");
    if (file.size() > 10 * 1024 * 1024) { // 10MB
        QString backupName = QString("logs/app_%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        file.copy(backupName);
        file.remove();
    }

    log(Info, "日志系统初始化完成");
}

QString Logger::levelToString(LogLevel level)
{
    switch (level) {
    case Debug: return "DEBUG";
    case Info: return "INFO";
    case Warning: return "WARNING";
    case Error: return "ERROR";
    default: return "UNKNOWN";
    }
}
