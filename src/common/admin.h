#ifndef ADMIN_H
#define ADMIN_H

#include <QString>
#include <QDateTime>
#include <QDebug>
#include "hashsha.h"
#include "desutil.h"
#include "user.h"  // 复用 User 的密码强度枚举和校验方法

/**
 * @enum AdminValidationError
 * @brief 管理员数据校验错误码
 */
enum AdminValidationError {
    ERR_ADMIN_OK = 0,                    // 校验通过
    ERR_ADMIN_USERNAME_EMPTY,            // 用户名为空
    ERR_ADMIN_USERNAME_LENGTH,           // 用户名长度不符
    ERR_ADMIN_USERNAME_INVALID_CHAR,     // 用户名包含非法字符
    ERR_ADMIN_PASSWORD_EMPTY,            // 密码为空
    ERR_ADMIN_PASSWORD_WEAK,             // 密码强度不足
    ERR_ADMIN_KEY_EMPTY,                 // 管理员密钥为空
    ERR_ADMIN_KEY_INVALID                // 管理员密钥无效
};

/**
 * @enum AdminRole
 * @brief 管理员角色枚举
 */
enum AdminRole {
    ADMIN_SUPER = 0,     // 超级管理员（全部权限）
    ADMIN_MANAGER = 1,   // 管理员（大部分权限）
    ADMIN_OPERATOR = 2   // 操作员（有限权限）
};

/**
 * @struct AdminPermission
 * @brief 管理员权限描述
 */
struct AdminPermission {
    bool canManageUsers = false;       // 管理用户
    bool canManageProducts = false;    // 管理商品
    bool canManageInventory = false;   // 管理库存
    bool canViewReports = false;       // 查看报表
    bool canManageAdmins = false;      // 管理管理员
    bool canManageFinance = false;     // 财务管理

    // 根据角色获取默认权限
    static AdminPermission fromRole(AdminRole role) {
        AdminPermission perm;
        switch (role) {
        case ADMIN_SUPER:
            perm.canManageUsers = true;
            perm.canManageProducts = true;
            perm.canManageInventory = true;
            perm.canViewReports = true;
            perm.canManageAdmins = true;
            perm.canManageFinance = true;
            break;
        case ADMIN_MANAGER:
            perm.canManageUsers = true;
            perm.canManageProducts = true;
            perm.canManageInventory = true;
            perm.canViewReports = true;
            perm.canManageAdmins = false;
            perm.canManageFinance = false;
            break;
        case ADMIN_OPERATOR:
            perm.canManageUsers = false;
            perm.canManageProducts = false;
            perm.canManageInventory = true;
            perm.canViewReports = true;
            perm.canManageAdmins = false;
            perm.canManageFinance = false;
            break;
        }
        return perm;
    }
};

/**
 * @class Admin
 * @brief 管理员实体类 - 封装管理员信息和权限管理逻辑
 */
class Admin
{
public:
    // ========== 构造与析构 ==========

    Admin();
    Admin(const QString& username, const QString& password, AdminRole role = ADMIN_OPERATOR);
    Admin(int id, const QString& username, const QString& passwordHash,
          AdminRole role, const AdminPermission& permission,
          bool isActive, const QDateTime& createdAt, const QDateTime& lastLogin);

    Admin(const Admin& other);
    Admin(Admin&& other) noexcept;
    ~Admin();

    // ========== 赋值运算符 ==========

    Admin& operator=(const Admin& other);
    Admin& operator=(Admin&& other) noexcept;

    // ========== Getter 方法 ==========

    int getId() const { return m_id; }
    QString getUsername() const { return m_username; }
    QString getPasswordHash() const { return m_passwordHash; }
    AdminRole getRole() const { return m_role; }
    QString getRoleName() const;
    AdminPermission getPermission() const { return m_permission; }
    bool isActive() const { return m_isActive; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getLastLogin() const { return m_lastLogin; }

    // ========== Setter 方法 ==========

    bool setUsername(const QString& username);
    bool setPassword(const QString& plainPassword);
    void setPasswordHash(const QString& hash);
    void setRole(AdminRole role);
    void setPermission(const AdminPermission& permission);
    void setActive(bool active) { m_isActive = active; }
    void setLastLogin(const QDateTime& lastLogin) { m_lastLogin = lastLogin; }
    void setCreatedAt(const QDateTime& createdAt) { m_createdAt = createdAt; }

    // ========== 权限检查方法 ==========

    bool canManageUsers() const { return m_permission.canManageUsers; }
    bool canManageProducts() const { return m_permission.canManageProducts; }
    bool canManageInventory() const { return m_permission.canManageInventory; }
    bool canViewReports() const { return m_permission.canViewReports; }
    bool canManageAdmins() const { return m_permission.canManageAdmins; }
    bool canManageFinance() const { return m_permission.canManageFinance; }
    bool hasPermission(const AdminPermission& required) const;

    // ========== 静态校验方法 ==========

    static bool validateAdminUsername(const QString& username, AdminValidationError* errorCode = nullptr);
    static bool validateAdminPassword(const QString& password, AdminValidationError* errorCode = nullptr);
    static bool validateAdminKey(const QString& key, AdminValidationError* errorCode = nullptr);
    static QString getErrorMessage(AdminValidationError errorCode);

    // ========== 实例方法 ==========

    bool verifyPassword(const QString& plainPassword) const;
    void clear();
    QString toString() const;
    bool equals(const Admin& other) const;

private:
    // ========== 成员变量 ==========
    int m_id;                       // 管理员ID（-1表示未持久化）
    QString m_username;             // 管理员用户名
    QString m_passwordHash;         // 密码哈希值
    AdminRole m_role;               // 管理员角色
    AdminPermission m_permission;   // 权限列表
    bool m_isActive;                // 是否激活
    QDateTime m_createdAt;          // 创建时间
    QDateTime m_lastLogin;          // 最后登录时间

    // ========== 校验规则常量 ==========
    static constexpr int MIN_USERNAME_LEN = 3;
    static constexpr int MAX_USERNAME_LEN = 20;
    static constexpr int MIN_PASSWORD_LEN = 8;
    static constexpr int MAX_PASSWORD_LEN = 32;

    // 默认管理员密钥（可配置）
    static const QString s_adminKey;
};

#endif // ADMIN_H
