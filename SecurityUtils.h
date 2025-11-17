#ifndef SECURITYUTILS_H
#define SECURITYUTILS_H

#include <QCryptographicHash>
#include <QString>

class SecurityUtils
{
public:
    static QString hashPassword(const QString& password);
    static bool verifyPassword(const QString& password, const QString& hash);

private:
    static QString generateSalt();
};


#endif // SECURITYUTILS_H
