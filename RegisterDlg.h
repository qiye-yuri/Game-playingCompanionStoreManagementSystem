#ifndef REGISTERDLG_H
#define REGISTERDLG_H

#include <QDialog>

namespace Ui {
class RegisterDlg;
}

class RegisterDlg : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDlg(QWidget *parent = nullptr);
    ~RegisterDlg();

private slots:
    void on_registerBtn_clicked();
    void on_cancelBtn_clicked();

private:
    Ui::RegisterDlg *ui;
    bool validateInput();
};

#endif // REGISTERDLG_H
