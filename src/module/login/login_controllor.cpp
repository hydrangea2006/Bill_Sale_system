#include "login_controllor.h"
#include <QDebug>

login_controllor::login_controllor(QObject *parent)
    : QObject(parent)
    , m_view(nullptr) // 初始化成员指针，防止野指针
{
    // 1. 尝试连接数据库
    // 因为 Controller 是业务逻辑的起点，如果数据库连不上，后面全是废话
    if (!DatabaseManager::instance().connectToDatabase()) {
        qCritical() << "致命错误：数据库初始化失败！" << DatabaseManager::instance().getLastError();
    } else {
        qDebug() << "Controller 已就绪：数据库连接成功。";
    }
}
void login_controllor::setView(LoginWidget *view) {
    m_view = view;
    connect(m_view, &LoginWidget::registerSubmitted, this, &login_controllor::onHandleRegister);
    connect(m_view, &LoginWidget::loginSubmitted,this,&login_controllor::onHandleLogin);
}
void login_controllor::onHandleRegister(const QString &u, const QString &e,
                                  const QString &ph, const QString &p,
                                  const QString &cp)
{
    if (p != cp) {
        m_view->showRegisterError("两次输入的密码不一致！");
        return;
    }

    UserValidationError err;
    // 质检
    if (!User::validateUsername(u, &err)) {
        m_view->showRegisterError(User::getErrorMessage(err));
        return;
    }
    // 查重
    if (DatabaseManager::instance().userExists(u)) {
        m_view->showRegisterError("该用户名已被占用");
        return;
    }

    QString safePwd = HashSha::hashSha256(p);
    // 注册并获取结果
    if (DatabaseManager::instance().registerUser(DESutil::encryptWithDefaultKey(u), e, ph, safePwd)) {
        qDebug() << "注册成功: " << u;
        m_view->showRegisterSuccess("注册成功！");
    } else {
        m_view->showRegisterError("注册失败：" + DatabaseManager::instance().getLastError());
    }
}

void login_controllor::onHandleLogin(const QString &username, const QString &password) {
    // 1. 去除两端空格，防止用户输入时不小心按了空格
    QString cleanAccount = username.trimmed();

    // 2. 统一使用 DatabaseManager 进行查询
    UserInfo info = DatabaseManager::instance().findUserByAccount(cleanAccount);

    if (info.id <= 0) {
        qDebug() << "结果: 数据库未找到匹配账号";
        m_view->showLoginError("账号不存在");
        return;
    }

    // 3. 校验哈希
    if (HashSha::hashSha256(password) == info.passwordHash) {
        qDebug() << "结果: 登录成功，用户ID:" << info.id;
        m_view->showLoginSuccess("登录成功");
    } else {
        qDebug() << "结果: 密码错误";
        m_view->showLoginError("密码错误");
    }
}