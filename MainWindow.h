#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QStandardItemModel>
#include <QMap>
#include <QTreeWidgetItem>

class Logger;
class ValidatorUtils;
class SecurityUtils;
class OrderSettlement;
class RechargeReviewDlg;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    enum NavType{
        Unknown,
        User_Search,
        User_Add,
        Order_View,
        Order_Create,
        Order_Accept,
        Order_Manage,
        User_Profile,
        Recharge_Review
    };

    MainWindow(QWidget *parent = nullptr);  // 添加构造函数
    ~MainWindow();  // 添加析构函数

    void setUserInfo(const QString& userId, const QString& role);
    void initNavTreeWidgetByRole();

private slots:
    void on_orderView();
    void on_orderCreate();
    void on_orderAccept();
    void on_orderManage();
    void on_userSearch();
    void on_userAdd();
    void on_userProfile();
    void on_treeItemClicked(QTreeWidgetItem* item, int column);
    void on_personalInfoClicked();

private:
    void createOrderPage();
    void createOrderAcceptPage();
    void createOrderManagePage();
    void createUserAddPage();
    void createUserProfilePage();
    void refreshBalance();
    void showPage(QWidget* page);
    void clearStackedWidget();
    void on_rechargeReview();

private:
    Ui::MainWindow *ui;
    QString m_userId;
    QString m_userRole;
    QStandardItemModel *m_user_model{};
    QStandardItemModel *m_order_model{};
    QMap<NavType, QWidget*> m_pageMap;
};
#endif // MAINWINDOW_H
