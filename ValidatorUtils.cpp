#include "ValidatorUtils.h"
#include <QRegularExpression>

bool ValidatorUtils::validateUserId(const QString& userId)
{
    // 用户ID: 4-20位字母数字
    QRegularExpression regex("^[a-zA-Z0-9]{4,20}$");
    return regex.match(userId).hasMatch();
}

bool ValidatorUtils::validatePassword(const QString& password)
{
    // 密码: 至少6位，包含字母和数字
    if(password.length() < 6) return false;

    bool hasLetter = false;
    bool hasDigit = false;

    for(const QChar& ch : password) {
        if(ch.isLetter()) hasLetter = true;
        if(ch.isDigit()) hasDigit = true;
    }

    return hasLetter && hasDigit;
}

bool ValidatorUtils::validatePrice(const QString& price)
{
    bool ok;
    double value = price.toDouble(&ok);
    return ok && value > 0;
}

bool ValidatorUtils::validateEmail(const QString& email)
{
    QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return regex.match(email).hasMatch();
}

bool ValidatorUtils::validatePhone(const QString& phone)
{
    QRegularExpression regex("^1[3-9]\\d{9}$");
    return regex.match(phone).hasMatch();
}
