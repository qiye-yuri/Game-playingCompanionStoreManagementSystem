#ifndef RECHARGEREVIEWDLG_H
#define RECHARGEREVIEWDLG_H

#include <QDialog>

namespace Ui {
class RechargeReviewDlg;
}

class RechargeReviewDlg : public QDialog
{
    Q_OBJECT

public:
    explicit RechargeReviewDlg(QWidget *parent = nullptr);
    ~RechargeReviewDlg();

    void loadPendingRecharges();

private slots:
    void on_approveBtn_clicked();
    void on_rejectBtn_clicked();
    void on_refreshBtn_clicked();
    void initializeTable();

private:
    Ui::RechargeReviewDlg *ui;
    int getSelectedRecordId();
};


#endif // RECHARGEREVIEWDLG_H
