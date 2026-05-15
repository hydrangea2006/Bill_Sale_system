#include <QApplication>
#include <QMetaObject>
#include <QMessageBox>
#include <QRandomGenerator>
#include "login_controllor.h"
#include "databasemanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!DatabaseManager::instance().connectToDatabase()) {
        qDebug() << "数据库连接失败：" << DatabaseManager::instance().getLastError();
        return -1;
    }
    qDebug() << "数据库连接成功";

    LoginWidget loginWidget;
    login_controllor con;
    con.setView(&loginWidget);
    loginWidget.show();
    return a.exec();
}