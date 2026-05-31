/**
 * @file mainwindow.cpp
 * @brief Password Reset Window Implementation: Handles password recovery interaction and feedback
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>

// Constructor: Initialize UI and set input text color (override dark theme)
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Force text color to black for clear visibility in all themes
    QPalette pal = ui->edit_account->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->edit_account->setPalette(pal);
    ui->edit_code->setPalette(pal);
    ui->edit_newPwd->setPalette(pal);
    ui->edit_confirmPwd->setPalette(pal);
}

// Destructor: Auto release UI resources
MainWindow::~MainWindow()
{
    delete ui;
}

// ========== Public Slots for Backend Calls (Controller Feedback Interface) ==========

// Receive reset failure message from controller
void MainWindow::showResetError(const QString &message)
{
    QMessageBox::warning(this, "Reset Failed", message);
}

// Receive reset success message from controller
void MainWindow::showResetSuccess(const QString &message)
{
    QMessageBox::information(this, "Reset Success", message);
    emit closed(); // Notify login window process finished
    this->close();
}

// ========== UI Interaction Logic ==========

// Back to Login: Close current window and notify parent
void MainWindow::on_btn_back_clicked()
{
    emit closed();
    this->close();
}

// Get Verification Code Logic
void MainWindow::on_btn_getCode_clicked()
{
    QString account = ui->edit_account->text().trimmed();

    // Empty check
    if (account.isEmpty()) {
        QMessageBox::warning(this, "Tip", "Please enter account/phone number");
        return;
    }

    // Key point: Emit signal to Controller. UI only handles requests, not logic
    qDebug() << ">>> [DEBUG] Emitting verification code request, account:" << account;
    emit requestVerificationCode(account);

    QMessageBox::information(this, "Tip", "Verification code request sent, please wait...");
}

// Submit Reset Password Logic
void MainWindow::on_btn_submit_clicked()
{
    QString account = ui->edit_account->text().trimmed();
    QString code = ui->edit_code->text().trimmed();
    QString newPwd = ui->edit_newPwd->text();
    QString confirmPwd = ui->edit_confirmPwd->text();

    // Frontend validation: Check complete input
    if (account.isEmpty() || code.isEmpty() || newPwd.isEmpty() || confirmPwd.isEmpty()) {
        QMessageBox::warning(this, "Tip", "Please fill in all information");
        return;
    }
    // Check password consistency
    if (newPwd != confirmPwd) {
        QMessageBox::warning(this, "Tip", "Passwords do not match");
        return;
    }

    // Key point: Send data to Controller for database reset
    qDebug() << ">>> [DEBUG] Emitting password reset signal...";
    emit resetPasswordRequested(account, code, newPwd, confirmPwd);
}

// ========== Input Restriction Logic ==========

// Limit account input to 11 digits (common phone length)
void MainWindow::on_edit_account_textChanged(const QString &text)
{
    if (text.length() > 11) {
        ui->edit_account->setText(text.left(11));
    }
}

// Limit verification code to numbers only
void MainWindow::on_edit_code_textChanged(const QString &text)
{
    if (!text.isEmpty() && !text.back().isDigit()) {
        ui->edit_code->setText(text.chopped(1));
    }
}

// Real-time check: Monitor password consistency and show visual tips
void MainWindow::on_edit_newPwd_textChanged(const QString &text)
{
    if (!ui->edit_confirmPwd->text().isEmpty()) {
        if (text != ui->edit_confirmPwd->text()) {
            ui->label_tip->setText("Passwords do not match");
        } else {
            ui->label_tip->clear();
        }
    }
}

void MainWindow::on_edit_confirmPwd_textChanged(const QString &text)
{
    if (text != ui->edit_newPwd->text()) {
        ui->label_tip->setText("Passwords do not match");
    } else {
        ui->label_tip->clear();
    }
}
