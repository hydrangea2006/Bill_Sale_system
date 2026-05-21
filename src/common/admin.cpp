#include "admin.h"
#include <QRegularExpression>
#include <utility>

// 定义管理员密钥常量
const QString Admin::s_adminKey = "Admin@2024!SecureKey";

// ===================== 构造、析构与拷贝控制 =====================

Admin::Admin()
    : m_id(-1)
    , m_role(ADMIN_OPERATOR)
    , m_isActive(true)
{
    m_permission = AdminPermission::fromRole(m_role);
}

Admin::Admin(const QString& username, const QString& password, AdminRole role)
    : m_id(-1)
    , m_role(role)
    , m_isActive(true)
{
    m_username = username;
    m_permission = AdminPermission::fromRole(role);
    setPassword(password);
}

Admin::Admin(int id, const QString& username, const QString& passwordHash,
             AdminRole role, const AdminPermission& permission,
             bool isActive, const QDateTime& createdAt, const QDateTime& lastLogin)
    : m_id(id)
    , m_username(username)
    , m_passwordHash(passwordHash)
    , m_role(role)
    , m_permission(permission)
    , m_isActive(isActive)
    , m_createdAt(createdAt)
    , m_lastLogin(lastLogin)
{
}

Admin::Admin(const Admin& other) = default;

Admin::Admin(Admin&& other) noexcept
    : m_id(std::exchange(other.m_id, -1))
    , m_username(std::move(other.m_username))
    , m_passwordHash(std::move(other.m_passwordHash))
    , m_role(other.m_role)
    , m_permission(other.m_permission)
    , m_isActive(other.m_isActive)
    , m_createdAt(std::move(other.m_createdAt))
    , m_lastLogin(std::move(other.m_lastLogin))
{
}

Admin::~Admin() = default;

// ===================== 赋值运算符 =====================

Admin& Admin::operator=(const Admin& other)
{
    if (this != &other) {
        m_id = other.m_id;
        m_username = other.m_username;
        m_passwordHash = other.m_passwordHash;
        m_role = other.m_role;
        m_permission = other.m_permission;
        m_isActive = other.m_isActive;
        m_createdAt = other.m_createdAt;
        m_lastLogin = other.m_lastLogin;
    }
    return *this;
}

Admin& Admin::operator=(Admin&& other) noexcept
{
    if (this != &other) {
        m_id = std::exchange(other.m_id, -1);
        m_username = std::move(other.m_username);
        m_passwordHash = std::move(other.m_passwordHash);
        m_role = other.m_role;
        m_permission = other.m_permission;
        m_isActive = other.m_isActive;
        m_createdAt = std::move(other.m_createdAt);
        m_lastLogin = std::move(other.m_lastLogin);
    }
    return *this;
}

// ===================== Getter 方法 =====================

QString Admin::getRoleName() const
{
    switch (m_role) {
    case ADMIN_SUPER:    return QStringLiteral("超级管理员");
    case ADMIN_MANAGER:  return QStringLiteral("管理员");
    case ADMIN_OPERATOR: return QStringLiteral("操作员");
    default:             return QStringLiteral("未知角色");
    }
}

// ===================== Setter 方法 =====================

bool Admin::setUsername(const QString& username)
{
    AdminValidationError err;
    if (validateAdminUsername(username, &err)) {
        m_username = username.trimmed();
        return true;
    }
    return false;
}

bool Admin::setPassword(const QString& plainPassword)
{
    AdminValidationError err;
    if (validateAdminPassword(plainPassword, &err)) {
        m_passwordHash = HashSha::hashSha256(plainPassword);
        return true;
    }
    return false;
}

void Admin::setPasswordHash(const QString& hash)
{
    m_passwordHash = hash;
}

void Admin::setRole(AdminRole role)
{
    m_role = role;
    m_permission = AdminPermission::fromRole(role);
}

void Admin::setPermission(const AdminPermission& permission)
{
    m_permission = permission;
}

// ===================== 权限检查方法 =====================

bool Admin::hasPermission(const AdminPermission& required) const
{
    if (m_role == ADMIN_SUPER) return true;

    if (required.canManageUsers && !m_permission.canManageUsers) return false;
    if (required.canManageProducts && !m_permission.canManageProducts) return false;
    if (required.canManageInventory && !m_permission.canManageInventory) return false;
    if (required.canViewReports && !m_permission.canViewReports) return false;
    if (required.canManageAdmins && !m_permission.canManageAdmins) return false;
    if (required.canManageFinance && !m_permission.canManageFinance) return false;

    return true;
}

// ===================== 静态校验方法 =====================

bool Admin::validateAdminUsername(const QString& username, AdminValidationError* errorCode)
{
    if (username.isEmpty()) {
        if (errorCode) *errorCode = ERR_ADMIN_USERNAME_EMPTY;
        return false;
    }
    if (username.length() < MIN_USERNAME_LEN || username.length() > MAX_USERNAME_LEN) {
        if (errorCode) *errorCode = ERR_ADMIN_USERNAME_LENGTH;
        return false;
    }
    // 只允许字母、数字、下划线、中文
    static QRegularExpression regex("^[a-zA-Z0-9_\\x{4e00}-\\x{9fa5}]+$");
    if (!regex.match(username).hasMatch()) {
        if (errorCode) *errorCode = ERR_ADMIN_USERNAME_INVALID_CHAR;
        return false;
    }
    return true;
}

bool Admin::validateAdminPassword(const QString& password, AdminValidationError* errorCode)
{
    if (password.isEmpty()) {
        if (errorCode) *errorCode = ERR_ADMIN_PASSWORD_EMPTY;
        return false;
    }
    if (password.length() < MIN_PASSWORD_LEN) {
        if (errorCode) *errorCode = ERR_ADMIN_PASSWORD_WEAK;
        return false;
    }
    // 简单强度检查：至少包含字母和数字
    bool hasLetter = false, hasDigit = false;
    for (const QChar& c : password) {
        if (c.isLetter()) hasLetter = true;
        else if (c.isDigit()) hasDigit = true;
    }
    if (!hasLetter || !hasDigit) {
        if (errorCode) *errorCode = ERR_ADMIN_PASSWORD_WEAK;
        return false;
    }
    return true;
}

bool Admin::validateAdminKey(const QString& key, AdminValidationError* errorCode)
{
    if (key.isEmpty()) {
        if (errorCode) *errorCode = ERR_ADMIN_KEY_EMPTY;
        return false;
    }
    if (key != s_adminKey) {
        if (errorCode) *errorCode = ERR_ADMIN_KEY_INVALID;
        return false;
    }
    return true;
}

QString Admin::getErrorMessage(AdminValidationError errorCode)
{
    switch (errorCode) {
    case ERR_ADMIN_OK:                 return QStringLiteral("校验通过");
    case ERR_ADMIN_USERNAME_EMPTY:     return QStringLiteral("用户名不能为空");
    case ERR_ADMIN_USERNAME_LENGTH:    return QStringLiteral("用户名长度应为3-20个字符");
    case ERR_ADMIN_USERNAME_INVALID_CHAR: return QStringLiteral("用户名只能包含中英文、数字和下划线");
    case ERR_ADMIN_PASSWORD_EMPTY:     return QStringLiteral("密码不能为空");
    case ERR_ADMIN_PASSWORD_WEAK:      return QStringLiteral("密码强度不足（至少8位，需包含字母和数字）");
    case ERR_ADMIN_KEY_EMPTY:          return QStringLiteral("管理员密钥不能为空");
    case ERR_ADMIN_KEY_INVALID:        return QStringLiteral("管理员密钥无效");
    default:                           return QStringLiteral("未知错误");
    }
}

// ===================== 实例方法 =====================

bool Admin::verifyPassword(const QString& plainPassword) const
{
    if (m_passwordHash.isEmpty() || plainPassword.isEmpty()) return false;
    return m_passwordHash == HashSha::hashSha256(plainPassword);
}

void Admin::clear()
{
    m_id = -1;
    m_username.clear();
    m_passwordHash.clear();
    m_role = ADMIN_OPERATOR;
    m_permission = AdminPermission::fromRole(ADMIN_OPERATOR);
    m_isActive = false;
    m_createdAt = QDateTime();
    m_lastLogin = QDateTime();
}

QString Admin::toString() const
{
    return QStringLiteral("Admin{id=%1, username='%2', role=%3, active=%4}")
        .arg(m_id)
        .arg(m_username)
        .arg(getRoleName())
        .arg(m_isActive ? QStringLiteral("是") : QStringLiteral("否"));
}

bool Admin::equals(const Admin& other) const
{
    return m_id == other.m_id &&
           m_username == other.m_username &&
           m_role == other.m_role &&
           m_isActive == other.m_isActive;
}
