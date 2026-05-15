#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "registerwidget.h"
#include "mainwindow.h"
#include <QMessageBox>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    // 设置用户名和密码输入框的文字颜色为黑色
    QPalette pal = ui->leUsername->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->leUsername->setPalette(pal);
    ui->lePassword->setPalette(pal);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

// ========== 后端调用的槽实现 ==========

void LoginWidget::showLoginError(const QString &message)
{
    QMessageBox::warning(this, "登录失败", message);
}

void LoginWidget::showLoginSuccess(const QString &message)
{
    QMessageBox::information(this, "登录成功", message);
    // TODO: 这里以后可以打开主程序界面
}

void LoginWidget::showRegisterError(const QString &message)
{
    if (m_regWindow) m_regWindow->showRegisterError(message);
}

void LoginWidget::showRegisterSuccess(const QString &message)
{
    if (m_regWindow) m_regWindow->showRegisterSuccess(message);
}

void LoginWidget::showResetError(const QString &message)
{
    if (m_forgetWindow) m_forgetWindow->showResetError(message);
}

void LoginWidget::showResetSuccess(const QString &message)
{
    if (m_forgetWindow) m_forgetWindow->showResetSuccess(message);
}

// ========== 忘记密码 ==========
void LoginWidget::on_btn_forgetPwd_clicked()
{
    this->hide();

    m_forgetWindow = new MainWindow(this);
    m_forgetWindow->setAttribute(Qt::WA_DeleteOnClose);

    // 子窗口销毁时清空指针，并恢复登录界面
    connect(m_forgetWindow, &MainWindow::destroyed, this, [this]() {
        m_forgetWindow = nullptr;
        this->show();
    });

    // 转发"提交重置"信号给后端
    connect(m_forgetWindow, &MainWindow::resetPasswordRequested,
            this, &LoginWidget::passwordResetSubmitted);

    m_forgetWindow->show();
}

// ========== 注册跳转 ==========
void LoginWidget::on_btn_register_clicked()
{
    this->hide();

    m_regWindow = new RegisterWidget();
    m_regWindow->setAttribute(Qt::WA_DeleteOnClose);

    // 子窗口销毁时清空指针，并恢复登录界面
    connect(m_regWindow, &RegisterWidget::destroyed, this, [this]() {
        m_regWindow = nullptr;
        this->show();
    });

    connect(m_regWindow, &RegisterWidget::backToLogin, this, [this]() {
        if (m_regWindow) m_regWindow->close();
    });

    // 转发"注册提交"信号给后端
    connect(m_regWindow, &RegisterWidget::registerSubmitted,
            this, &LoginWidget::registerSubmitted);

    m_regWindow->show();
}

// ========== 登录按钮 ==========
void LoginWidget::on_btnLogin_clicked()
{
    QString username = ui->leUsername->text().trimmed();
    QString password = ui->lePassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码");
        return;
    }

    // 发射信号，后端接手验证
    emit loginSubmitted(username, password);
}