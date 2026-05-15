#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H
#include <QWidget>

namespace Ui {
    class LoginWidget;
}

class RegisterWidget;
class MainWindow;

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

    // ========== 后端调用的槽（UI 反馈接口） ==========
public slots:
    // 登录相关
    void showLoginError(const QString &message);
    void showLoginSuccess(const QString &message);

    // 注册相关（LoginWidget 会转发给当前打开的 RegisterWidget）
    void showRegisterError(const QString &message);
    void showRegisterSuccess(const QString &message);

    // 找回密码相关（LoginWidget 会转发给当前打开的 MainWindow）
    void showResetError(const QString &message);
    void showResetSuccess(const QString &message);

    signals:
        // ========== 向后端发射的业务信号 ==========
        void loginSubmitted(const QString &username, const QString &password);

    // 注册数据（用户名、邮箱、手机号、密码、确认密码）
    void registerSubmitted(const QString &username,
                           const QString &email,
                           const QString &phone,
                           const QString &password,
                           const QString &confirmPassword);

    // 找回密码：提交重置（账号、验证码、新密码、确认密码）
    void passwordResetSubmitted(const QString &account,
                                const QString &code,
                                const QString &newPassword,
                                const QString &confirmPassword);

private slots:
    void on_btn_forgetPwd_clicked();
    void on_btn_register_clicked();
    void on_btnLogin_clicked();

private:
    Ui::LoginWidget *ui;

    // 持有当前打开的子窗口指针，用于向后端"转发"指令
    RegisterWidget *m_regWindow = nullptr;
    MainWindow     *m_forgetWindow = nullptr;
};

#endif