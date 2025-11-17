#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>
#include <QString>

class ForgotPasswordDlg;
class RegisterDlg;

namespace Ui {
class LoginDlg;
}

class LoginDlg : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDlg(QWidget *parent = nullptr);
    ~LoginDlg();

    QString getCurrentUserId() const { return m_currentUserId; }
    QString getCurrentUserRole() const { return m_currentUserRole; }

protected slots:
    void on_loginBtn_clicked();
    void on_guestLoginBtn_clicked();
    void on_registerBtn_clicked();
    void handleForgotPassword();
    bool validateLoginInput(const QString& userId, const QString& password);
    bool authenticateUser(const QString& userId, const QString& password, QString& errorMsg);
    void handleGuestLogin();

private:
    Ui::LoginDlg *ui;
    QString m_currentUserId;
    QString m_currentUserRole;
    ForgotPasswordDlg *m_forgotPasswordDlg;
    RegisterDlg *m_registerDlg;
};

#endif // LOGINDLG_H
