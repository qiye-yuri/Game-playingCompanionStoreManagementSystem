#ifndef VALIDATORUTILS_H
#define VALIDATORUTILS_H

#include <QString>

class ValidatorUtils
{
public:
    static bool validateUserId(const QString& userId);
    static bool validatePassword(const QString& password);
    static bool validateEmail(const QString& email);
    static bool validatePhone(const QString& phone);
    static bool validatePrice(const QString& price);
};

#endif // VALIDATORUTILS_H
