#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>

namespace Ui {
    class RegisterWidget;
}

class RegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = nullptr);
    ~RegisterWidget();

    signals:
        void backToLogin(); // 点击返回登录

    // 注册数据（用户名、邮箱、手机号、密码、确认密码）
    void registerSubmitted(const QString &username,
                           const QString &email,
                           const QString &phone,
                           const QString &password,
                           const QString &confirmPassword);

public slots:
    // 后端调用：显示错误 / 成功
    void showRegisterError(const QString &message);
    void showRegisterSuccess(const QString &message);

private slots:
    void on_btn_BackToLogin_clicked();
    void on_btn_RegisterSubmit_clicked();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::RegisterWidget *ui;
};

#endif