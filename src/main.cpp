#include <QApplication>
#include <QMetaObject>
#include <QMessageBox>
#include <QRandomGenerator>
#include "login_controllor.h"
#include "databasemanager.h"
#include "module/inventory/inventorywidget.h"

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

    // === 临时：直接显示库存界面===
    InventoryWidget inventoryWidget;
    inventoryWidget.show();

    return a.exec();
}