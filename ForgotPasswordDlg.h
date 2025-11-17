#ifndef FORGOTPASSWORDDLG_H
#define FORGOTPASSWORDDLG_H

#include <QDialog>
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QDateTime>

namespace Ui {
class ForgotPasswordDlg;
}

class ForgotPasswordDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ForgotPasswordDlg(QWidget *parent = nullptr);
    ~ForgotPasswordDlg();

private slots:
    void on_resetBtn_clicked();
    void on_cancelBtn_clicked();


private:
    Ui::ForgotPasswordDlg *ui;
    bool validateInput();
    bool verifyUserIdentity();
};

#endif // FORGOTPASSWORDDLG_H
