#include "emailservice.h"
#include <QRandomGenerator>
#include <QDebug>

QMap<QString, EmailService::CodeInfo> EmailService::s_codes;

QString EmailService::generateCode()
{
    // 生成6位随机数字验证码
    int code = QRandomGenerator::global()->bounded(100000, 999999);
    return QString::number(code);
}

QString EmailService::sendVerificationCode(const QString &email)
{
    if (email.isEmpty()) return {};

    QString code = generateCode();

    // 存储验证码，有效期5分钟
    CodeInfo info;
    info.code = code;
    info.expireTime = QDateTime::currentDateTime().addSecs(300);
    s_codes[email] = info;

    // 实际发送邮件（需要配置SMTP服务器）
    // 这里提供接口框架，生产环境可接入：
    //
    //   1. SMTP直连（需服务器地址、端口、账号密码）
    //      QSmtpClient smtp;
    //      smtp.connectToHost("smtp.qq.com", 465);
    //      smtp.login("your@qq.com", "password");
    //      smtp.sendMail("your@qq.com", email, "验证码", "您的验证码是: " + code);
    //
    //   2. 第三方邮件API（SendGrid、Mailgun等）
    //      QNetworkAccessManager mgr;
    //      QNetworkRequest req(QUrl("https://api.sendgrid.com/v3/mail/send"));
    //      req.setRawHeader("Authorization", "Bearer API_KEY");
    //      // ... 发送JSON请求
    //
    //   3. Windows下也可调用系统mailto协议:
    //      QDesktopServices::openUrl(QUrl("mailto:" + email + "?subject=验证码&body=您的验证码是: " + code));

    qDebug() << "[邮箱验证] 向" << email << "发送验证码:" << code
             << "(有效期5分钟，实际发送需配置SMTP)";

    return code;
}

bool EmailService::verifyCode(const QString &email, const QString &code)
{
    cleanExpiredCodes();

    auto it = s_codes.find(email);
    if (it == s_codes.end()) return false;
    if (it.value().code != code) return false;
    if (QDateTime::currentDateTime() > it.value().expireTime) return false;

    // 验证成功后删除验证码（一次性使用）
    s_codes.erase(it);
    return true;
}

void EmailService::cleanExpiredCodes()
{
    QDateTime now = QDateTime::currentDateTime();
    for (auto it = s_codes.begin(); it != s_codes.end(); ) {
        if (now > it.value().expireTime) {
            it = s_codes.erase(it);
        } else {
            ++it;
        }
    }
}
