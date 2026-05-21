#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QList>
#include "desutil.h"

struct UserInfo {
    int id = -1;
    QString username;
    QString email;
    QString phone;
    QString passwordHash;
    bool isActive = true;
    QString createdAt;
};

struct ProductRecord {
    int id = -1;
    QString name;
    double purchasePrice = 0.0;
    double salePrice = 0.0;
    int quantity = 0;
    QString unit;
    QString remark;
    QString updatedAt;
};

struct SalesOrderRecord {
    int id = -1;
    int userId = 0;
    QString address;
    double totalAmount = 0.0;
    QString remark;
    QString createdAt;
};

struct SalesOrderItemRecord {
    int id = -1;
    int orderId = 0;
    int productId = 0;
    int quantity = 0;
    double unitPrice = 0.0;
    double subtotal = 0.0;
};

class DatabaseManager
{
public:
    // 单例访问
    static DatabaseManager& instance();

    // 数据库连接
    bool connectToDatabase();
    bool connectToSQLite();
    void disconnectDatabase();
    bool isConnected() const;
    QString getLastError() const;

    // 用户查找
    UserInfo findUserByAccount(const QString& account);
    bool userExists(const QString& account);
    int getUserIdByAccount(const QString& account);

    // 用户注册
    bool registerUser(const QString& username, const QString& email,
                      const QString& phone, const QString& passwordHash,
                      int* outUserId = nullptr);

    // 密码更新
    bool updatePassword(int userId, const QString& newPasswordHash);
    bool updatePassword(const QString& account, const QString& newPasswordHash);
    //精确删除用户
    bool deleteUserByAccount(const QString& account);

    // ========== 商品管理 CRUD ==========
    int addProduct(const QString& name, double purchasePrice, double salePrice,
                   const QString& unit, int quantity);
    bool updateProduct(int productId, const QString& name, double purchasePrice,
                       double salePrice, const QString& unit, const QString& remark);
    bool deleteProduct(int productId);
    bool updateProductStock(int productId, int newQuantity);
    QList<ProductRecord> getAllProducts();
    QList<ProductRecord> searchProducts(const QString& keyword);
    ProductRecord getProductById(int productId);

    // ========== 订单相关 ==========
    int createOrder(int userId, const QString& address, const QList<SalesOrderItemRecord>& items);
    QList<SalesOrderRecord> getUserOrders(int userId);
    QList<SalesOrderItemRecord> getOrderItems(int orderId);

    // ========== 库存扣减 ==========
    bool deductProductStock(int productId, int quantity);

    // ========== 账户余额 ==========
    double getBalance();
    bool rechargeBalance(double amount);
    bool deductBalance(double amount);

    // ========== 收货地址 ==========
    int addAddress(int userId, const QString& name, const QString& phone,
                   const QString& province, const QString& city,
                   const QString& district, const QString& detail,
                   bool isDefault = false);
    bool updateAddress(int id, const QString& name, const QString& phone,
                       const QString& province, const QString& city,
                       const QString& district, const QString& detail,
                       bool isDefault = false);
    bool deleteAddress(int id);
    bool setDefaultAddress(int userId, int addressId);
    struct AddressRecord {
        int id = -1;
        int userId = 0;
        QString name;
        QString phone;
        QString province;
        QString city;
        QString district;
        QString detail;
        bool isDefault = false;
        QString createdAt;
    };
    QList<AddressRecord> getUserAddresses(int userId);

    // ========== 流水记录 ==========
    struct TransactionRecord {
        int id = -1;
        int type = 0;        // 0=支出, 1=收入
        double amount = 0.0;
        QString remark;
        QString createdAt;
    };
    bool addTransaction(int type, double amount, const QString& remark);
    QList<TransactionRecord> getAllTransactions();

    // 工具
    static QString hashSha256(const QString& input);

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool initDatabase();

    // 内部 SQL 执行
    bool execute(const QString& sql, const QVariantList& params = QVariantList());
    QVariant getSingleValue(const QString& sql, const QVariantList& params = QVariantList());
    QVariantMap getSingleRecord(const QString& sql, const QVariantList& params = QVariantList());

    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_connected;
    QString m_lastError;
};

#endif // DATABASEMANAGER_HABASEMANAGER_H