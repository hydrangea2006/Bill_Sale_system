#ifndef EMAILSERVICE_H
#define EMAILSERVICE_H

#include <QString>
#include <QMap>
#include <QDateTime>

class EmailService
{
public:
    // 生成并发送验证码到指定邮箱
    // 返回验证码（失败返回空字符串）
    static QString sendVerificationCode(const QString &email);

    // 验证验证码是否正确且未过期（有效期5分钟）
    static bool verifyCode(const QString &email, const QString &code);

    // 清除过期验证码
    static void cleanExpiredCodes();

private:
    EmailService() = delete;

    struct CodeInfo {
        QString code;
        QDateTime expireTime;
    };

    // 存储验证码：key=邮箱, value=验证码信息
    static QMap<QString, CodeInfo> s_codes;

    // 生成6位随机数字验证码
    static QString generateCode();
};

#endif // EMAILSERVICE_H
