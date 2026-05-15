#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariantMap>
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

#endif // DATABASEMANAGER_H