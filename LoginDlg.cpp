#include "LoginDlg.h"
#include "ui_LoginDlg.h"
#include "DatabaseManager.h"
#include "SecurityUtils.h"
#include "ValidatorUtils.h"
#include "Logger.h"
#include "ForgotPasswordDlg.h"
#include "RegisterDlg.h"

#include<QMessageBox>
#include<QSqlQuery>
#include<QSqlError>
#include<QDateTime>


LoginDlg::LoginDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDlg)
    , m_forgotPasswordDlg(nullptr)
    , m_registerDlg(nullptr)
{
    ui->setupUi(this);

    // 初始化数据库连接
    if(!DatabaseManager::instance().openDatabase()) {
        QMessageBox::critical(this, "系统错误", "数据库连接失败，系统无法启动");
        reject();
        return;
    }
    connect(ui->forgotpasswordBtn, &QPushButton::clicked, this, &LoginDlg::handleForgotPassword);

    Logger::log(Logger::Info, "登录界面初始化完成");
}

LoginDlg::~LoginDlg()
{
    delete ui;
}

void LoginDlg::on_loginBtn_clicked()
{
    QString userId = ui->accountEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    Logger::log(Logger::Info, QString("尝试登录 - 用户ID: %1").arg(userId));

    // 输入验证
    if(!validateLoginInput(userId, password)) {
        Logger::log(Logger::Warning, QString("登录输入验证失败 - 用户ID: %1").arg(userId));
        return;
    }
    QString errorMsg;

    // 验证用户凭据
    if(authenticateUser(userId, password, errorMsg)) {
        Logger::log(Logger::Info, QString("用户登录成功: %1, 角色: %2").arg(userId).arg(m_currentUserRole), userId);
        accept();
    } else {
        Logger::log(Logger::Warning, QString("登录失败: %1, 原因: %2").arg(userId).arg(errorMsg));
        QMessageBox::warning(this, "登录失败", errorMsg);
    }
}

void LoginDlg::on_guestLoginBtn_clicked()
{
    Logger::log(Logger::Info, "游客登录尝试");
    handleGuestLogin();
}

void LoginDlg::on_registerBtn_clicked()
{
    if(!m_registerDlg) {
        m_registerDlg = new RegisterDlg(this);
    }

    if(m_registerDlg->exec() == QDialog::Accepted) {
        // 注册成功，清空输入框
        ui->accountEdit->clear();
        ui->passwordEdit->clear();
    }
}

bool LoginDlg::validateLoginInput(const QString& userId, const QString& password)
{
    if(userId.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空");
        return false;
    }

    if(!ValidatorUtils::validateUserId(userId)) {
        QMessageBox::warning(this, "输入错误", "用户名格式不正确");
        return false;
    }

    return true;
}

bool LoginDlg::authenticateUser(const QString& userId, const QString& password, QString& errorMsg)
{
    QSqlQuery query;
    query.prepare("SELECT user_id, password, role FROM users WHERE user_id = ?");
    query.bindValue(0, userId);

    if(!query.exec()) {
        errorMsg = "数据库查询失败: " + query.lastError().text();
        return false;
    }

    if(!query.next()) {
        errorMsg = "用户不存在";
        return false;
    }

    QString storedHash = query.value(1).toString();

    if(SecurityUtils::verifyPassword(password, storedHash)) {
        m_currentUserId = query.value(0).toString();
        m_currentUserRole = query.value(2).toString();
        return true;
    }

    errorMsg = "密码错误";
    return false;
}

void LoginDlg::handleGuestLogin()
{
    // 创建临时游客账号
    QString guestId = "guest_" + QString::number(QDateTime::currentSecsSinceEpoch());

    // 检查是否已存在游客账号，如果存在则复用
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT user_id FROM users WHERE user_id LIKE 'guest_%' AND role = 'guest' LIMIT 1");

    if(checkQuery.exec() && checkQuery.next()) {
        m_currentUserId = checkQuery.value(0).toString();
        Logger::log(Logger::Info, QString("复用现有游客账号: %1").arg(m_currentUserId));
    } else {
        // 创建新的游客账号
        m_currentUserId = guestId;
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO users (user_id, password, role, name, created_at) VALUES (?, ?, 'guest', ?, NOW())");
        insertQuery.bindValue(0, m_currentUserId);
        insertQuery.bindValue(1, SecurityUtils::hashPassword("guest123")); // 默认密码
        insertQuery.bindValue(2, "游客用户");

        if(!insertQuery.exec()) {
            Logger::log(Logger::Error, QString("创建游客账号失败: %1").arg(insertQuery.lastError().text()));
            QMessageBox::warning(this, "游客登录", "游客登录失败: " + insertQuery.lastError().text());
            return;
        }
        Logger::log(Logger::Info, QString("创建新游客账号: %1").arg(m_currentUserId));
    }

    m_currentUserRole = "guest";
    Logger::log(Logger::Info, QString("游客登录成功: %1").arg(m_currentUserId), m_currentUserId);
    done(QDialog::Accepted);
}

void LoginDlg::handleForgotPassword()
{
    if(!m_forgotPasswordDlg) {
        m_forgotPasswordDlg = new ForgotPasswordDlg(this);
    }

    m_forgotPasswordDlg->exec();
}
