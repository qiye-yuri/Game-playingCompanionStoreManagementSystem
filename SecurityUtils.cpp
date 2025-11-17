#include "SecurityUtils.h"
#include <QRandomGenerator>
#include <QStringList>

QString SecurityUtils::hashPassword(const QString& password)
{
    // 使用SHA-256哈希
    QString salt = generateSalt();
    QByteArray data = (salt + password).toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return salt + "$" + QString(hash.toHex());
}

QString SecurityUtils::generateSalt()
{
    const QString possibleChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString salt;
    for(int i = 0; i < 16; ++i) {
        salt.append(possibleChars.at(QRandomGenerator::global()->bounded(possibleChars.length())));
    }
    return salt;
}

bool SecurityUtils::verifyPassword(const QString& password, const QString& hash)
{
    QStringList parts = hash.split("$");
    if(parts.size() != 2) return false;

    QString salt = parts[0];
    QByteArray data = (salt + password).toUtf8();
    QByteArray computedHash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString(computedHash.toHex()) == parts[1];
}
