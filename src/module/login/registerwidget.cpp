/**
 * @file registerwidget.cpp
 * @brief Registration Window Implementation: responsible for UI display, input validation and registration signal emission
 */

#include "registerwidget.h"
#include "ui_registerwidget.h"
#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

// Constructor: Initialize window style, role distinction and input field visual settings
RegisterWidget::RegisterWidget(int role, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWidget)
    , m_role(role) // Save user role (0: Normal User, 1: Administrator)
{
    ui->setupUi(this);
    // Auto delete on close to prevent memory leaks
    this->setAttribute(Qt::WA_DeleteOnClose);

    // Set borderless and transparent window for modern custom UI
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Add shadow effect to the card for better layering
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 50));
    shadow->setOffset(0, 5);
    ui->cardWidget->setGraphicsEffect(shadow);

    // Dynamically set title and button color based on role
    if (role == 1) {
        ui->label_title->setText("👑 Admin Register");
        ui->btn_RegisterSubmit->setStyleSheet(
            "QPushButton { background-color: #e6a23c; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: #d4912e; }"
            );
    } else {
        ui->label_title->setText("📝 User Register");
        ui->btn_RegisterSubmit->setStyleSheet(
            "QPushButton { background-color: #67c23a; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: #5daf34; }"
            );
    }

    // Force input text to black to avoid being overwritten by global dark style
    QPalette pal = ui->le_Register_Username->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->le_Register_Username->setPalette(pal);
    ui->le_Register_Email->setPalette(pal);
    ui->le_Register_Phone->setPalette(pal);
    ui->le_Register_Password->setPalette(pal);
    ui->le_Register_ConfirmPwd->setPalette(pal);

    // Connect close button slot
    connect(ui->btn_close, &QPushButton::clicked, this, &RegisterWidget::on_btn_close_clicked);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

// Close window slot function
void RegisterWidget::on_btn_close_clicked()
{
    this->close();
}

// Show event: Center window on screen every time it opens
void RegisterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen->availableGeometry();
    move(screenGeo.center() - rect().center());
}

// Mouse press event: Enable window drag via title bar area
void RegisterWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        QWidget *child = childAt(pos);
        // Record offset if clicking title bar area
        if (child == ui->label_title || pos.y() < 80) {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

// Mouse move event: Move window in real-time based on mouse offset
void RegisterWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !m_dragPosition.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

// Error message interface called by controller
void RegisterWidget::showRegisterError(const QString &message)
{
    QMessageBox::warning(this, "Register Failed", message);
}

// Success handler interface called by controller
void RegisterWidget::showRegisterSuccess(const QString &message)
{
    QMessageBox::information(this, "Register Success", message);
    emit backToLogin(); // Signal: notify login page to return
    this->close();
}

// Back to login button logic
void RegisterWidget::on_btn_BackToLogin_clicked()
{
    emit backToLogin(); // Signal: request back to login page
    this->close();
}

// Submit registration: Frontend data validation layer to ensure valid data for logic layer
void RegisterWidget::on_btn_RegisterSubmit_clicked()
{
    // Get and trim input content
    QString username = ui->le_Register_Username->text().trimmed();
    QString email    = ui->le_Register_Email->text().trimmed();
    QString phone    = ui->le_Register_Phone->text().trimmed();
    QString password = ui->le_Register_Password->text();
    QString confirm  = ui->le_Register_ConfirmPwd->text();

    // Format validation logic
    if (username.isEmpty()) { QMessageBox::warning(this, "Tip", "Please enter username"); return; }
    if (username.length() < 3 || username.length() > 20) { QMessageBox::warning(this, "Tip", "Username must be 3-20 characters"); return; }
    if (email.isEmpty()) { QMessageBox::warning(this, "Tip", "Please enter email"); return; }
    if (!email.contains('@') || !email.contains('.')) { QMessageBox::warning(this, "Tip", "Please enter a valid email address"); return; }
    if (password.isEmpty()) { QMessageBox::warning(this, "Tip", "Please enter password"); return; }
    if (password.length() < 6 || password.length() > 20) { QMessageBox::warning(this, "Tip", "Password must be 6-20 characters"); return; }
    if (password != confirm) { QMessageBox::warning(this, "Tip", "Passwords do not match"); return; }

    // Core signal: Emit all validated registration data to controller layer
    emit registerSubmitted(username, email, phone, password, confirm, m_role);
}
