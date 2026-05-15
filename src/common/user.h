#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>
#include "hashsha.h"
#include "desutil.h"
/**
 * @enum PasswordStrength
 * @brief 密码强度枚举
 */
enum PasswordStrength {
    PASSWORD_WEAK = 0,          // 弱：长度不足或纯数字/字母
    PASSWORD_MEDIUM = 1,        // 中：长度足够，数字+字母组合
    PASSWORD_STRONG = 2,        // 强：包含大小写字母+数字
    PASSWORD_VERY_STRONG = 3    // 极强：包含大小写+数字+特殊字符
};

/**
 * @enum UserValidationError
 * @brief 用户数据校验错误码
 */
enum UserValidationError {
    ERR_VALID_OK = 0,                       // 校验通过
    ERR_USERNAME_EMPTY,                     // 用户名为空
    ERR_USERNAME_LENGTH,                    // 用户名长度不符
    ERR_USERNAME_INVALID_CHAR,              // 用户名包含非法字符
    ERR_EMAIL_EMPTY,                        // 邮箱为空
    ERR_EMAIL_INVALID,                      // 邮箱格式无效
    ERR_PHONE_INVALID,                      // 手机号格式无效
    ERR_PASSWORD_EMPTY,                     // 密码为空
    ERR_PASSWORD_WEAK,                      // 密码强度不足
    ERR_PASSWORD_HASH_EMPTY                 // 密码哈希为空
};

/**
 * @class User
 * @brief 用户实体类 - 封装用户信息和数据校验逻辑
 */
class User
{
public:
    // ========== 构造与析构 ==========

    /**
     * @brief 默认构造函数
     * 使用初始化列表初始化所有成员变量为默认值
     */
    User();

    /**
     * @brief 带参构造函数（注册时使用）
     * @param username 用户名
     * @param email 邮箱
     * @param phone 手机号（可选）
     * @param password 明文密码（会自动哈希）
     */
    User(const QString& username, const QString& email,
         const QString& phone, const QString& password);

    /**
     * @brief 完整构造函数（从数据库加载时使用）
     */
    User(int id, const QString& username, const QString& email,
         const QString& phone, const QString& passwordHash,
         bool isActive, const QDateTime& createdAt,
         const QDateTime& lastLogin);

    /**
     * @brief 拷贝构造函数
     */
    User(const User& other);

    /**
     * @brief 移动构造函数
     */
    User(User&& other) noexcept;

    /**
     * @brief 析构函数 - 安全释放资源
     */
    ~User();

    // ========== 赋值运算符 ==========

    User& operator=(const User& other);
    User& operator=(User&& other) noexcept;

    // ========== Getter 方法 ==========

    int getId() const { return m_id; }
    QString getUsername() const { return m_username; }
    QString getEmail() const { return m_email; }
    QString getPhone() const { return m_phone; }
    QString getPasswordHash() const { return m_passwordHash; }
    bool isActive() const { return m_isActive; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getLastLogin() const { return m_lastLogin; }

    // ========== Setter 方法（带完整校验） ==========

    bool setUsername(const QString& username);
    bool setEmail(const QString& email);
    bool setPhone(const QString& phone);
    bool setPassword(const QString& plainPassword);
    bool setPasswordWithStrength(const QString& plainPassword, PasswordStrength minStrength);
    void setPasswordHash(const QString& hash);
    void setActive(bool active) { m_isActive = active; }
    void setLastLogin(const QDateTime& lastLogin) { m_lastLogin = lastLogin; }
    void setCreatedAt(const QDateTime& createdAt) { m_createdAt = createdAt; }

    // ========== 静态数据校验方法（完整算法） ==========

    static bool validateUsername(const QString& username, UserValidationError* errorCode = nullptr);
    static bool validateEmail(const QString& email, UserValidationError* errorCode = nullptr);
    static bool validatePhone(const QString& phone, bool allowEmpty = true,
                              UserValidationError* errorCode = nullptr);
    static PasswordStrength checkPasswordStrength(const QString& password);
    static PasswordStrength checkPasswordStrengthDetailed(const QString& password,
                                                          bool& hasUpper,
                                                          bool& hasLower,
                                                          bool& hasDigit,
                                                          bool& hasSpecial);

    // ========== 实例数据校验方法 ==========

    bool validateAll(UserValidationError* errorCode = nullptr);
    bool validateAllWithMessage(QString* errorMessage = nullptr);
    bool verifyPassword(const QString& plainPassword) const;

    // ========== 工具方法 ==========

    void clear();
    QString toString() const;
    bool equals(const User& other, int compareBy = 0) const;
    static QString getErrorMessage(UserValidationError errorCode);

private:
    // ========== 成员变量（m_ 前缀） ==========
    int m_id;                   // 用户ID（-1表示未持久化）
    QString m_username;         // 用户名
    QString m_email;            // 邮箱
    QString m_phone;            // 手机号
    QString m_passwordHash;     // 密码哈希值
    bool m_isActive;            // 账号是否激活
    QDateTime m_createdAt;      // 注册时间
    QDateTime m_lastLogin;      // 最后登录时间

    // ========== 静态校验规则常量 ==========
    static constexpr int MIN_USERNAME_LEN = 3;
    static constexpr int MAX_USERNAME_LEN = 20;
    static constexpr int MIN_PASSWORD_LEN = 8;
    static constexpr int PHONE_LEN = 11;

    // 正则表达式模式（编译一次，多次使用）
    static QRegularExpression s_usernameRegex;
    static QRegularExpression s_emailRegex;
    static QRegularExpression s_phoneRegex;

    // 初始化正则表达式
    static void initRegex();
};

#endif // USER_H