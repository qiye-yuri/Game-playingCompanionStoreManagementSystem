#include "MainWindow.h"
#include "LoginDlg.h"
#include "Logger.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Logger::setupLogger();
    Logger::log(Logger::Info, "应用程序启动");

    LoginDlg loginDlg;
    if(loginDlg.exec() == QDialog::Accepted) {
        MainWindow w;
        w.setUserInfo(loginDlg.getCurrentUserId(), loginDlg.getCurrentUserRole());
        w.show();

        Logger::log(Logger::Info,
                    QString("用户进入主界面 - 用户ID: %1, 角色: %2")
                        .arg(loginDlg.getCurrentUserId())
                        .arg(loginDlg.getCurrentUserRole()),
                    loginDlg.getCurrentUserId());

        return a.exec();
    }

    Logger::log(Logger::Info, "应用程序退出");
    return 0;
}
