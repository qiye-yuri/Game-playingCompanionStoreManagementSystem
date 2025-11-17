#include "ForgotPasswordDlg.h"
#include "ui_ForgotPasswordDlg.h"
#include "DatabaseManager.h"
#include "SecurityUtils.h"
#include "ValidatorUtils.h"
#include "Logger.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

ForgotPasswordDlg::ForgotPasswordDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ForgotPasswordDlg)
{
    ui->setupUi(this);
}

ForgotPasswordDlg::~ForgotPasswordDlg()
{
    delete ui;
}

void ForgotPasswordDlg::on_resetBtn_clicked()
{
    if(!validateInput()) {
        return;
    }

    if(!verifyUserIdentity()) {
        QMessageBox::warning(this, "身份验证", "用户ID或邮箱不正确");
        Logger::log(Logger::Warning, QString("密码重置身份验证失败 - 用户ID: %1, 邮箱: %2")
                                         .arg(ui->userIdEdit->text()).arg(ui->emailEdit->text()));
        return;
    }

    QString userId = ui->userIdEdit->text().trimmed();
    QString newPassword = ui->newPasswordEdit->text();
    QString hashedPassword = SecurityUtils::hashPassword(newPassword);

    // 更新密码
    QSqlQuery query;
    query.prepare("UPDATE users SET password = ? WHERE user_id = ? AND email = ?");
    query.bindValue(0, hashedPassword);
    query.bindValue(1, userId);
    query.bindValue(2, ui->emailEdit->text().trimmed());

    if(query.exec()) {
        if(query.numRowsAffected() > 0) {
            Logger::log(Logger::Info, QString("密码重置成功 - 用户ID: %1").arg(userId), userId);
            QMessageBox::information(this, "密码重置", "密码重置成功，请使用新密码登录");
            accept();
        } else {
            Logger::log(Logger::Error, QString("密码重置失败 - 未找到匹配的用户 - 用户ID: %1").arg(userId));
            QMessageBox::warning(this, "密码重置", "重置失败，请检查用户信息");
        }
    } else {
        Logger::log(Logger::Error, QString("密码重置数据库错误: %1 - 用户ID: %2")
                                       .arg(query.lastError().text()).arg(userId));
        QMessageBox::warning(this, "密码重置", "重置失败: " + query.lastError().text());
    }
}

void ForgotPasswordDlg::on_cancelBtn_clicked()
{
    reject();
}

bool ForgotPasswordDlg::validateInput()
{
    QString userId = ui->userIdEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();
    QString newPassword = ui->newPasswordEdit->text();
    QString confirmPassword = ui->confirmPasswordEdit->text();

    if(userId.isEmpty() || email.isEmpty() || newPassword.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "所有字段都不能为空");
        return false;
    }

    if(!ValidatorUtils::validateUserId(userId)) {
        QMessageBox::warning(this, "输入错误", "用户ID格式不正确");
        return false;
    }

    if(!ValidatorUtils::validateEmail(email)) {
        QMessageBox::warning(this, "输入错误", "邮箱格式不正确");
        return false;
    }

    if(!ValidatorUtils::validatePassword(newPassword)) {
        QMessageBox::warning(this, "输入错误", "密码格式不正确（至少6位，包含字母和数字）");
        return false;
    }

    if(newPassword != confirmPassword) {
        QMessageBox::warning(this, "输入错误", "两次输入的密码不一致");
        return false;
    }

    return true;
}

bool ForgotPasswordDlg::verifyUserIdentity()
{
    QString userId = ui->userIdEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();

    QSqlQuery query;
    query.prepare("SELECT user_id FROM users WHERE user_id = ? AND email = ?");
    query.bindValue(0, userId);
    query.bindValue(1, email);

    if(query.exec() && query.next()) {
        return true;
    }

    return false;
}
