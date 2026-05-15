#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // ===== 在这里添加以下代码 =====
    QPalette pal = ui->edit_account->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->edit_account->setPalette(pal);
    ui->edit_code->setPalette(pal);
    ui->edit_newPwd->setPalette(pal);
    ui->edit_confirmPwd->setPalette(pal);
    // ===== 添加结束 =====
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ========== 后端调用的槽 ==========
void MainWindow::showResetError(const QString &message)
{
    QMessageBox::warning(this, "重置失败", message);
}

void MainWindow::showResetSuccess(const QString &message)
{
    QMessageBox::information(this, "重置成功", message);
    emit closed();
    this->close();
}

// ========== 返回登录 ==========
void MainWindow::on_btn_back_clicked()
{
    emit closed();
    this->close();
}

// ========== 获取验证码 ==========
void MainWindow::on_btn_getCode_clicked()
{
    QString account = ui->edit_account->text().trimmed();
    if (account.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入账号/手机号");
        return;
    }
    // 前端只校验非空，具体发验证码逻辑由后端处理
    // 如需通知后端，可在此处增加信号发射
    QMessageBox::information(this, "提示", "验证码已发送（模拟: 123456）");
}

// ========== 提交找回 ==========
void MainWindow::on_btn_submit_clicked()
{
    QString account = ui->edit_account->text().trimmed();
    QString code = ui->edit_code->text().trimmed();
    QString newPwd = ui->edit_newPwd->text();
    QString confirmPwd = ui->edit_confirmPwd->text();

    // 基础校验
    if (account.isEmpty() || code.isEmpty() || newPwd.isEmpty() || confirmPwd.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写完整信息");
        return;
    }
    if (newPwd != confirmPwd) {
        QMessageBox::warning(this, "提示", "两次输入的密码不一致");
        return;
    }
    if (newPwd.length() < 6) {
        QMessageBox::warning(this, "提示", "新密码长度不能少于6位");
        return;
    }

    // 发射信号，后端接手验证验证码并重置密码
    emit resetPasswordRequested(account, code, newPwd, confirmPwd);
}

// ========== 输入限制（保持原有）==========
void MainWindow::on_edit_account_textChanged(const QString &text)
{
    if (text.length() > 11) {
        ui->edit_account->setText(text.left(11));
    }
}

void MainWindow::on_edit_code_textChanged(const QString &text)
{
    if (!text.isEmpty() && !text.back().isDigit()) {
        ui->edit_code->setText(text.chopped(1));
    }
}

void MainWindow::on_edit_newPwd_textChanged(const QString &text)
{
    if (!ui->edit_confirmPwd->text().isEmpty()) {
        if (text != ui->edit_confirmPwd->text()) {
            ui->label_tip->setText("两次密码不一致");
        } else {
            ui->label_tip->clear();
        }
    }
}

void MainWindow::on_edit_confirmPwd_textChanged(const QString &text)
{
    if (text != ui->edit_newPwd->text()) {
        ui->label_tip->setText("两次密码不一致");
    } else {
        ui->label_tip->clear();
    }
}