#include "RegisterDlg.h"
#include "ui_RegisterDlg.h"
#include "DatabaseManager.h"
#include "SecurityUtils.h"
#include "ValidatorUtils.h"
#include "Logger.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

RegisterDlg::RegisterDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDlg)
{
    ui->setupUi(this);
}

RegisterDlg::~RegisterDlg()
{
    delete ui;
}

void RegisterDlg::on_registerBtn_clicked()
{
    if(!validateInput()) {
        return;
    }

    QString user_id = ui->userIdEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString name = ui->nameEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();

    // 检查用户是否已存在
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT user_id FROM users WHERE user_id = ? OR email = ?");
    checkQuery.bindValue(0, user_id);
    checkQuery.bindValue(1, email);

    if(checkQuery.exec()) {
        if(checkQuery.next()) {
            QMessageBox::warning(this, "注册", "用户ID或邮箱已存在");
            return;
        }
    } else {
        QMessageBox::warning(this, "注册", "数据库查询失败: " + checkQuery.lastError().text());
        return;
    }

    QString hashedPassword = SecurityUtils::hashPassword(password);

    // 注册新用户
    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO users (user_id, password, role, name, email, balance) VALUES (?, ?, 'user', ?, ?, 0.00)");
    insertQuery.bindValue(0, user_id);
    insertQuery.bindValue(1, hashedPassword);
    insertQuery.bindValue(2, name);
    insertQuery.bindValue(3, email);

    if(insertQuery.exec()) {
        Logger::log(Logger::Info, QString("用户注册成功: %1").arg(user_id), user_id);
        QMessageBox::information(this, "注册", "注册成功，请登录");
        accept();
    } else {
        Logger::log(Logger::Error, QString("注册失败: %1").arg(insertQuery.lastError().text()));
        QMessageBox::warning(this, "注册", "注册失败: " + insertQuery.lastError().text());
    }
}

void RegisterDlg::on_cancelBtn_clicked()
{
    reject();
}

bool RegisterDlg::validateInput()
{
    QString user_id = ui->userIdEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString confirmPassword = ui->confirmPasswordEdit->text();
    QString name = ui->nameEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();

    if(user_id.isEmpty() || password.isEmpty() || name.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "所有字段都不能为空");
        return false;
    }

    if(!ValidatorUtils::validateUserId(user_id)) {
        QMessageBox::warning(this, "输入错误", "用户ID格式不正确（4-20位字母数字）");
        return false;
    }

    if(!ValidatorUtils::validatePassword(password)) {
        QMessageBox::warning(this, "输入错误", "密码格式不正确（至少6位，包含字母和数字）");
        return false;
    }

    if(password != confirmPassword) {
        QMessageBox::warning(this, "输入错误", "两次输入的密码不一致");
        return false;
    }

    if(!ValidatorUtils::validateEmail(email)) {
        QMessageBox::warning(this, "输入错误", "邮箱格式不正确");
        return false;
    }

    return true;
}
