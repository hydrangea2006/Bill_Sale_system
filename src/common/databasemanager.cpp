#include "databasemanager.h"
#include "desutil.h"
#include <QCoreApplication>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDebug>

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
    QString createUsersTable =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username VARCHAR(50) NOT NULL UNIQUE,"
        "  email VARCHAR(100) NOT NULL UNIQUE,"
        "  phone VARCHAR(20) DEFAULT NULL,"
        "  password_hash VARCHAR(255) NOT NULL,"
        "  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "  is_active INTEGER DEFAULT 1"
        ")";

    if (!execute(createUsersTable)) return false;
    execute("CREATE INDEX IF NOT EXISTS idx_users_username ON users(username)");
    return true;
}

// 5. 通用执行函数
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