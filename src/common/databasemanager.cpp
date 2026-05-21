#include "databasemanager.h"
#include "desutil.h"
#include "hashsha.h"
#include <QCoreApplication>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

// 唯一的连接名，确保单例连接不冲突
static const char* DB_CONNECTION_NAME = "forgot_password_connection";

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}
// 1. 单例模式实现：解决你 main.cpp 中的 undefined reference 报错
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager _instance;
    return _instance;
}

DatabaseManager::DatabaseManager() : m_connected(false)
{
    if (QSqlDatabase::contains(DB_CONNECTION_NAME)) {
        m_db = QSqlDatabase::database(DB_CONNECTION_NAME);
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECTION_NAME);
    }
}

// 2. 状态检查
bool DatabaseManager::isConnected() const
{
    return m_connected && m_db.isOpen();
}

QString DatabaseManager::getLastError() const
{
    return m_db.lastError().text();
}

// 3. 数据库连接：统一命名为 connectToDatabase
bool DatabaseManager::connectToDatabase()
{
    if (isConnected()) return true;

    m_dbPath = QCoreApplication::applicationDirPath() + "/password.db";
    m_db.setDatabaseName(m_dbPath);

    if (m_db.open()) {
        m_connected = true;
        // 开启 WAL 模式提高并发性能
        QSqlQuery walQuery(m_db);
        walQuery.exec("PRAGMA journal_mode=WAL");
        return initDatabase();
    }
    return false;
}

// 4. 初始化表结构
bool DatabaseManager::initDatabase()
{
    QFile sqlFile(QCoreApplication::applicationDirPath() + "/../src/common/sql/bill_sale.sql");
    if (!sqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "无法打开 SQL 文件";
        return false;
    }

    QString sql = QTextStream(&sqlFile).readAll();
    sqlFile.close();

    // 移除注释，按分号分割执行
    sql.replace(QRegularExpression("--[^\n]*"), "");
    for (const QString& stmt : sql.split(';', Qt::SkipEmptyParts)) {
        QString trimmed = stmt.simplified();
        if (!trimmed.isEmpty() && !execute(trimmed)) {
            qCritical() << "执行 SQL 失败:" << trimmed;
            return false;
        }
    }

    qDebug() << "数据库表初始化成功";

    // 检查 accounts 表是否有初始记录，没有则插入
    QVariant val = getSingleValue("SELECT COUNT(*) FROM accounts");
    if (val.isValid() && val.toInt() == 0) {
        execute("INSERT INTO accounts (name, balance) VALUES ('主账户', 10000.00)");
        qDebug() << "已插入初始账户余额: ¥10000.00";
    }

    return true;
}

// 5. 通用查询：获取单个值
QVariant DatabaseManager::getSingleValue(const QString& sql, const QVariantList& params)
{
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& arg : params) {
        query.addBindValue(arg);
    }
    if (query.exec() && query.next()) {
        return query.value(0);
    }
    return QVariant();
}

// 5a. 通用查询：获取单条记录
QVariantMap DatabaseManager::getSingleRecord(const QString& sql, const QVariantList& params)
{
    QVariantMap record;
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& arg : params) {
        query.addBindValue(arg);
    }
    if (query.exec() && query.next()) {
        QSqlRecord qRecord = query.record();
        for (int i = 0; i < qRecord.count(); ++i) {
            record.insert(qRecord.fieldName(i), query.value(i));
        }
    }
    return record;
}

// 6. 通用执行函数
bool DatabaseManager::execute(const QString& sql, const QList<QVariant>& args)
{
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& arg : args) {
        query.addBindValue(arg);
    }
    if (!query.exec()) {
        qDebug() << "SQL Error:" << m_db.lastError().text();
        return false;
    }
    return true;
}

// 6. 查重逻辑：注册前调用
bool DatabaseManager::userExists(const QString& username)
{
    QSqlQuery query(m_db);
    // 数据库存的是密文，所以查重也要用密文去查
    query.prepare("SELECT 1 FROM users WHERE username = ?");
    query.addBindValue(username);
    return query.exec() && query.next();
}

// 7. 核心查询：findUserByAccount
UserInfo DatabaseManager::findUserByAccount(const QString& account)
{
    UserInfo info;
    // 步骤 A: 把用户输入的明文变成密文，才能去数据库里匹配
    QString encryptedSearch = DESutil::encryptWithDefaultKey(account.trimmed());

    QSqlQuery query(m_db);
    query.prepare("SELECT id, username, email, phone, password_hash, is_active "
                  "FROM users WHERE username = ? OR email = ? OR phone = ?");
    query.addBindValue(encryptedSearch);
    query.addBindValue(encryptedSearch);
    query.addBindValue(encryptedSearch);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();

        // 步骤 B: 数据库取出的是密文，这里使用解密还原成明文给 UI 使用
        // 这就是你刚才纠结的那行逻辑，应该放在“取出数据”之后
        info.username = DESutil::decryptWithDefaultKey(query.value("username").toString());
        info.email = DESutil::decryptWithDefaultKey(query.value("email").toString());

        info.passwordHash = query.value("password_hash").toString(); // Hash不需要解密
        info.isActive = query.value("is_active").toInt();

        qDebug() << "成功查获并解密用户:" << info.username;
    }
    return info;
}

// ===================== 密码更新 =====================

bool DatabaseManager::updatePassword(int userId, const QString& newPasswordHash)
{
    if (!isConnected()) return false;
    return execute("UPDATE users SET password_hash = ? WHERE id = ?",
                   {newPasswordHash, userId});
}

bool DatabaseManager::updatePassword(const QString& account, const QString& newPasswordHash)
{
    if (!isConnected()) return false;
    QString encrypted = DESutil::encryptWithDefaultKey(account.trimmed());
    return execute("UPDATE users SET password_hash = ? "
                   "WHERE username = ? OR email = ? OR phone = ?",
                   {newPasswordHash, encrypted, encrypted, encrypted});
}

QString DatabaseManager::hashSha256(const QString& input)
{
    return HashSha::hashSha256(input);
}

// 8. 写入操作
bool DatabaseManager::registerUser(const QString& username, const QString& email,
                                   const QString& phone, const QString& passwordHash,
                                   int* outUserId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (username, email, phone, password_hash) VALUES (?, ?, ?, ?)");
    query.addBindValue(username); // 这里的参数已经在 Controller 里加密过了
    query.addBindValue(email);
    query.addBindValue(phone);
    query.addBindValue(passwordHash);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (m_db.commit()) {
        if (outUserId) *outUserId = query.lastInsertId().toInt();
        return true;
    }
    return false;
}
// ===================== 商品管理 CRUD =====================

int DatabaseManager::addProduct(const QString& name, double purchasePrice,
                                 double salePrice, const QString& unit, int quantity)
{
    if (!isConnected()) return -1;

    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO products (name, purchase_price, sale_price, unit) VALUES (?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(purchasePrice);
    query.addBindValue(salePrice);
    query.addBindValue(unit);
    if (!query.exec()) {
        m_db.rollback();
        return -1;
    }

    int productId = query.lastInsertId().toInt();

    QSqlQuery invQuery(m_db);
    invQuery.prepare("INSERT INTO inventory (product_id, quantity) VALUES (?, ?)");
    invQuery.addBindValue(productId);
    invQuery.addBindValue(quantity);
    if (!invQuery.exec()) {
        m_db.rollback();
        return -1;
    }

    if (m_db.commit()) {
        return productId;
    }
    return -1;
}

bool DatabaseManager::updateProduct(int productId, const QString& name,
                                     double purchasePrice, double salePrice,
                                     const QString& unit, const QString& remark)
{
    if (!isConnected()) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE products SET name = ?, purchase_price = ?, sale_price = ?, "
                  "unit = ?, remark = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(purchasePrice);
    query.addBindValue(salePrice);
    query.addBindValue(unit);
    query.addBindValue(remark);
    query.addBindValue(productId);
    return query.exec();
}

bool DatabaseManager::deleteProduct(int productId)
{
    if (!isConnected()) return false;

    m_db.transaction();

    // 先删库存
    QSqlQuery invQuery(m_db);
    invQuery.prepare("DELETE FROM inventory WHERE product_id = ?");
    invQuery.addBindValue(productId);
    if (!invQuery.exec()) {
        m_db.rollback();
        return false;
    }

    // 再删商品
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM products WHERE id = ?");
    query.addBindValue(productId);
    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

bool DatabaseManager::updateProductStock(int productId, int newQuantity)
{
    if (!isConnected()) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE inventory SET quantity = ?, updated_at = CURRENT_TIMESTAMP "
                  "WHERE product_id = ?");
    query.addBindValue(newQuantity);
    query.addBindValue(productId);
    return query.exec();
}

QList<ProductRecord> DatabaseManager::getAllProducts()
{
    QList<ProductRecord> products;
    if (!isConnected()) return products;

    QSqlQuery query(m_db);
    query.prepare("SELECT p.id, p.name, p.purchase_price, p.sale_price, "
                  "p.unit, p.remark, i.quantity, p.updated_at "
                  "FROM products p LEFT JOIN inventory i ON p.id = i.product_id "
                  "ORDER BY p.id");

    if (query.exec()) {
        while (query.next()) {
            ProductRecord rec;
            rec.id = query.value("id").toInt();
            rec.name = query.value("name").toString();
            rec.purchasePrice = query.value("purchase_price").toDouble();
            rec.salePrice = query.value("sale_price").toDouble();
            rec.quantity = query.value("quantity").toInt();
            rec.unit = query.value("unit").toString();
            rec.remark = query.value("remark").toString();
            rec.updatedAt = query.value("updated_at").toDateTime().toString("yyyy-MM-dd");
            products.append(rec);
        }
    }
    return products;
}

QList<ProductRecord> DatabaseManager::searchProducts(const QString& keyword)
{
    QList<ProductRecord> products;
    if (!isConnected()) return products;

    QSqlQuery query(m_db);
    query.prepare("SELECT p.id, p.name, p.purchase_price, p.sale_price, "
                  "p.unit, p.remark, i.quantity, p.updated_at "
                  "FROM products p LEFT JOIN inventory i ON p.id = i.product_id "
                  "WHERE p.name LIKE ? ORDER BY p.id");
    query.addBindValue("%" + keyword + "%");

    if (query.exec()) {
        while (query.next()) {
            ProductRecord rec;
            rec.id = query.value("id").toInt();
            rec.name = query.value("name").toString();
            rec.purchasePrice = query.value("purchase_price").toDouble();
            rec.salePrice = query.value("sale_price").toDouble();
            rec.quantity = query.value("quantity").toInt();
            rec.unit = query.value("unit").toString();
            rec.remark = query.value("remark").toString();
            rec.updatedAt = query.value("updated_at").toDateTime().toString("yyyy-MM-dd");
            products.append(rec);
        }
    }
    return products;
}

// ===================== 库存扣减 =====================

bool DatabaseManager::deductProductStock(int productId, int quantity)
{
    if (!isConnected()) return false;

    QSqlQuery query(m_db);
    // 当前库存减 quantity，最低为0
    query.prepare("UPDATE inventory SET quantity = MAX(0, quantity - ?), "
                  "updated_at = CURRENT_TIMESTAMP WHERE product_id = ?");
    query.addBindValue(quantity);
    query.addBindValue(productId);
    return query.exec();
}

// ===================== 账户余额 =====================

double DatabaseManager::getBalance()
{
    if (!isConnected()) return 0.0;
    QVariant val = getSingleValue("SELECT balance FROM accounts WHERE id = 1");
    return val.isValid() ? val.toDouble() : 0.0;
}

bool DatabaseManager::rechargeBalance(double amount)
{
    if (!isConnected() || amount <= 0) return false;
    return execute("UPDATE accounts SET balance = balance + ?, "
                   "updated_at = CURRENT_TIMESTAMP WHERE id = 1",
                   {amount});
}

bool DatabaseManager::deductBalance(double amount)
{
    if (!isConnected()) return false;
    // 扣减余额，余额不能为负数
    return execute("UPDATE accounts SET balance = MAX(0, balance - ?), "
                   "updated_at = CURRENT_TIMESTAMP WHERE id = 1",
                   {amount});
}

// ===================== 流水记录 =====================

bool DatabaseManager::addTransaction(int type, double amount, const QString& remark)
{
    if (!isConnected()) return false;
    return execute("INSERT INTO transactions (type, amount, remark) VALUES (?, ?, ?)",
                   {type, amount, remark});
}

QList<DatabaseManager::TransactionRecord> DatabaseManager::getAllTransactions()
{
    QList<TransactionRecord> records;
    if (!isConnected()) return records;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, type, amount, remark, created_at "
                  "FROM transactions ORDER BY created_at DESC");

    if (query.exec()) {
        while (query.next()) {
            TransactionRecord r;
            r.id = query.value("id").toInt();
            r.type = query.value("type").toInt();
            r.amount = query.value("amount").toDouble();
            r.remark = query.value("remark").toString();
            r.createdAt = query.value("created_at").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
            records.append(r);
        }
    }
    return records;
}

// ===================== 订单相关 =====================

int DatabaseManager::createOrder(int userId, const QString& address,
                                  const QList<SalesOrderItemRecord>& items)
{
    if (!isConnected() || items.isEmpty()) return -1;

    m_db.transaction();

    // 1. 计算总金额
    double total = 0.0;
    for (const auto& item : items) {
        total += item.subtotal;
    }

    // 2. 插入 sales_orders
    QSqlQuery orderQuery(m_db);
    orderQuery.prepare("INSERT INTO sales_orders (user_id, address, total_amount) "
                       "VALUES (?, ?, ?)");
    orderQuery.addBindValue(userId);
    orderQuery.addBindValue(address);
    orderQuery.addBindValue(total);
    if (!orderQuery.exec()) {
        m_db.rollback();
        return -1;
    }
    int orderId = orderQuery.lastInsertId().toInt();

    // 3. 插入 sales_order_items + 扣减库存
    for (const auto& item : items) {
        // 插入明细
        QSqlQuery itemQuery(m_db);
        itemQuery.prepare("INSERT INTO sales_order_items (order_id, product_id, "
                          "quantity, unit_price, subtotal) VALUES (?, ?, ?, ?, ?)");
        itemQuery.addBindValue(orderId);
        itemQuery.addBindValue(item.productId);
        itemQuery.addBindValue(item.quantity);
        itemQuery.addBindValue(item.unitPrice);
        itemQuery.addBindValue(item.subtotal);
        if (!itemQuery.exec()) {
            m_db.rollback();
            return -1;
        }

        // 扣减库存
        if (!deductProductStock(item.productId, item.quantity)) {
            m_db.rollback();
            return -1;
        }
    }

    // 4. 扣减余额
    if (!deductBalance(total)) {
        m_db.rollback();
        return -1;
    }

    // 5. 记支出流水
    if (!addTransaction(0, total, QString("订单#%1").arg(orderId))) {
        m_db.rollback();
        return -1;
    }

    if (m_db.commit()) {
        return orderId;
    }
    m_db.rollback();
    return -1;
}

QList<SalesOrderRecord> DatabaseManager::getUserOrders(int userId)
{
    QList<SalesOrderRecord> orders;
    if (!isConnected()) return orders;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, user_id, address, total_amount, remark, created_at "
                  "FROM sales_orders WHERE user_id = ? ORDER BY created_at DESC");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            SalesOrderRecord r;
            r.id = query.value("id").toInt();
            r.userId = query.value("user_id").toInt();
            r.address = query.value("address").toString();
            r.totalAmount = query.value("total_amount").toDouble();
            r.remark = query.value("remark").toString();
            r.createdAt = query.value("created_at").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
            orders.append(r);
        }
    }
    return orders;
}

QList<SalesOrderItemRecord> DatabaseManager::getOrderItems(int orderId)
{
    QList<SalesOrderItemRecord> items;
    if (!isConnected()) return items;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, order_id, product_id, quantity, unit_price, subtotal "
                  "FROM sales_order_items WHERE order_id = ?");
    query.addBindValue(orderId);

    if (query.exec()) {
        while (query.next()) {
            SalesOrderItemRecord r;
            r.id = query.value("id").toInt();
            r.orderId = query.value("order_id").toInt();
            r.productId = query.value("product_id").toInt();
            r.quantity = query.value("quantity").toInt();
            r.unitPrice = query.value("unit_price").toDouble();
            r.subtotal = query.value("subtotal").toDouble();
            items.append(r);
        }
    }
    return items;
}

// ===================== 收货地址 =====================

int DatabaseManager::addAddress(int userId, const QString& name,
                                 const QString& phone,
                                 const QString& province, const QString& city,
                                 const QString& district, const QString& detail,
                                 bool isDefault)
{
    if (!isConnected()) return -1;

    // 如果是默认地址，先把该用户其他地址取消默认
    if (isDefault) {
        execute("UPDATE buyer_addresses SET is_default = 0 "
                "WHERE user_id = ? AND is_default = 1", {userId});
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO buyer_addresses (user_id, name, phone, "
                  "province, city, district, detail, is_default) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(name);
    query.addBindValue(phone);
    query.addBindValue(province);
    query.addBindValue(city);
    query.addBindValue(district);
    query.addBindValue(detail);
    query.addBindValue(isDefault ? 1 : 0);

    if (query.exec()) {
        return query.lastInsertId().toInt();
    }
    return -1;
}

bool DatabaseManager::updateAddress(int id, const QString& name,
                                     const QString& phone,
                                     const QString& province, const QString& city,
                                     const QString& district, const QString& detail,
                                     bool isDefault)
{
    if (!isConnected()) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE buyer_addresses SET name = ?, phone = ?, "
                  "province = ?, city = ?, district = ?, detail = ?, "
                  "is_default = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(phone);
    query.addBindValue(province);
    query.addBindValue(city);
    query.addBindValue(district);
    query.addBindValue(detail);
    query.addBindValue(isDefault ? 1 : 0);
    query.addBindValue(id);
    return query.exec();
}

bool DatabaseManager::deleteAddress(int id)
{
    if (!isConnected()) return false;
    return execute("UPDATE buyer_addresses SET is_active = 0 WHERE id = ?", {id});
}

bool DatabaseManager::setDefaultAddress(int userId, int addressId)
{
    if (!isConnected()) return false;
    m_db.transaction();
    // 取消该用户所有默认地址
    if (!execute("UPDATE buyer_addresses SET is_default = 0 "
                  "WHERE user_id = ? AND is_default = 1", {userId})) {
        m_db.rollback();
        return false;
    }
    // 设置新的默认地址
    if (!execute("UPDATE buyer_addresses SET is_default = 1 WHERE id = ?", {addressId})) {
        m_db.rollback();
        return false;
    }
    return m_db.commit();
}

QList<DatabaseManager::AddressRecord> DatabaseManager::getUserAddresses(int userId)
{
    QList<AddressRecord> records;
    if (!isConnected()) return records;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, user_id, name, phone, province, city, "
                  "district, detail, is_default, created_at "
                  "FROM buyer_addresses WHERE user_id = ? AND is_active = 1 "
                  "ORDER BY is_default DESC, created_at DESC");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            AddressRecord r;
            r.id = query.value("id").toInt();
            r.userId = query.value("user_id").toInt();
            r.name = query.value("name").toString();
            r.phone = query.value("phone").toString();
            r.province = query.value("province").toString();
            r.city = query.value("city").toString();
            r.district = query.value("district").toString();
            r.detail = query.value("detail").toString();
            r.isDefault = query.value("is_default").toInt() == 1;
            r.createdAt = query.value("created_at").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
            records.append(r);
        }
    }
    return records;
}

ProductRecord DatabaseManager::getProductById(int productId)
{
    ProductRecord rec;
    if (!isConnected()) return rec;

    QSqlQuery query(m_db);
    query.prepare("SELECT p.id, p.name, p.purchase_price, p.sale_price, "
                  "p.unit, p.remark, i.quantity, p.updated_at "
                  "FROM products p LEFT JOIN inventory i ON p.id = i.product_id "
                  "WHERE p.id = ?");
    query.addBindValue(productId);

    if (query.exec() && query.next()) {
        rec.id = query.value("id").toInt();
        rec.name = query.value("name").toString();
        rec.purchasePrice = query.value("purchase_price").toDouble();
        rec.salePrice = query.value("sale_price").toDouble();
        rec.quantity = query.value("quantity").toInt();
        rec.unit = query.value("unit").toString();
        rec.remark = query.value("remark").toString();
        rec.updatedAt = query.value("updated_at").toDateTime().toString("yyyy-MM-dd");
    }
    return rec;
}
