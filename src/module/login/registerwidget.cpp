#include "registerwidget.h"
#include "ui_registerwidget.h"
#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWidget)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    // 设置所有输入框的文字颜色为黑色
    QPalette pal = ui->le_Register_Username->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->le_Register_Username->setPalette(pal);
    ui->le_Register_Email->setPalette(pal);
    ui->le_Register_Phone->setPalette(pal);
    ui->le_Register_Password->setPalette(pal);
    ui->le_Register_ConfirmPwd->setPalette(pal);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

void RegisterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen->availableGeometry();
    move(screenGeo.center() - rect().center());
}

// ========== 后端调用的槽 ==========
void RegisterWidget::showRegisterError(const QString &message)
{
    QMessageBox::warning(this, "注册失败", message);
}

void RegisterWidget::showRegisterSuccess(const QString &message)
{
    QMessageBox::information(this, "注册成功", message);
    emit backToLogin();
    this->close();
}

// ========== 返回登录 ==========
void RegisterWidget::on_btn_BackToLogin_clicked()
{
    emit backToLogin();
    this->close();
}

// ========== 注册提交 ==========
void RegisterWidget::on_btn_RegisterSubmit_clicked()
{
    QString username = ui->le_Register_Username->text().trimmed();
    QString email    = ui->le_Register_Email->text().trimmed();
    QString phone    = ui->le_Register_Phone->text().trimmed();
    QString password = ui->le_Register_Password->text();
    QString confirm  = ui->le_Register_ConfirmPwd->text();

    // 前端基础校验
    if (username.isEmpty() || email.isEmpty() || password.isEmpty() || confirm.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写完整信息");
        return;
    }
    if (password != confirm) {
        QMessageBox::warning(this, "提示", "两次输入的密码不一致");
        return;
    }
    if (password.length() < 6 || password.length() > 20) {
        QMessageBox::warning(this, "提示", "密码长度需在6-20位之间");
        return;
    }

    // 发射信号，后端接手
    emit registerSubmitted(username, email, phone, password, confirm);
}