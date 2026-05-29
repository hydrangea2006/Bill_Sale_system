/**
 * @file loginwidget.cpp
 * @brief Login Window Implementation
 *
 * This file implements all functions of the login window, including:
 * - UI initialization and style settings
 * - Window drag functionality
 * - Interface navigation for login, register, and forget password
 * - Signal forwarding mechanism (transmit user operations to external business layer)
 */

#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "registerwidget.h"
#include "mainwindow.h"
#include "module/homewidget.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

// ==================== Constructor ====================
/**
 * @brief Login window constructor
 * @param parent Parent window pointer, default nullptr
 *
 * Functions:
 * 1. Load UI file and initialize interface
 * 2. Set borderless and transparent window style
 * 3. Add card shadow effect
 * 4. Set text color of input fields
 * 5. Connect signals and slots (Enter key login, close button)
 */
LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    // ========== Window Style Settings ==========
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // ========== Card Shadow Effect ==========
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 50));
    shadow->setOffset(0, 5);
    ui->cardWidget->setGraphicsEffect(shadow);

    // ========== Input Field Text Color ==========
    QPalette pal = ui->leUsername->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->leUsername->setPalette(pal);
    ui->lePassword->setPalette(pal);

    // ========== Signal-Slot Connections ==========
    connect(ui->lePassword, &QLineEdit::returnPressed,
            this, &LoginWidget::on_btnLogin_clicked);
    connect(ui->leUsername, &QLineEdit::returnPressed,
            this, &LoginWidget::on_btnLogin_clicked);
    connect(ui->btn_close, &QPushButton::clicked,
            this, &LoginWidget::on_btn_close_clicked);
}

// ==================== Destructor ====================
/**
 * @brief Login window destructor
 *
 * Function: Release memory occupied by UI object
 */
LoginWidget::~LoginWidget()
{
    delete ui;
}

// ==================== Private Slot Functions ====================

/**
 * @brief Close button click response
 *
 * Function: Exit the entire application
 */
void LoginWidget::on_btn_close_clicked()
{
    QApplication::quit();
}

/**
 * @brief Forget password button click response
 */
void LoginWidget::on_btn_forgetPwd_clicked()
{
    this->hide();

    m_forgetWindow = new MainWindow(this);
    m_forgetWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(m_forgetWindow, &MainWindow::destroyed, this, [this]() {
        m_forgetWindow = nullptr;
        this->show();
    });

    connect(m_forgetWindow, &MainWindow::requestVerificationCode,
            this, &LoginWidget::requestVerificationCode);

    connect(m_forgetWindow, &MainWindow::resetPasswordRequested,
            this, &LoginWidget::passwordResetSubmitted);

    m_forgetWindow->show();
}

/**
 * @brief User register button click response
 */
void LoginWidget::on_btn_register_clicked()
{
    this->hide();

    m_regWindow = new RegisterWidget(0);
    m_regWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(m_regWindow, &RegisterWidget::destroyed, this, [this]() {
        m_regWindow = nullptr;
        this->show();
    });

    connect(m_regWindow, &RegisterWidget::backToLogin, this, [this]() {
        if (m_regWindow)
            m_regWindow->close();
    });

    connect(m_regWindow, &RegisterWidget::registerSubmitted,
            this, &LoginWidget::registerSubmitted);

    m_regWindow->show();
}

/**
 * @brief Admin register button click response
 */
void LoginWidget::on_btn_registerAdmin_clicked()
{
    this->hide();

    m_regWindow = new RegisterWidget(1);
    m_regWindow->setAttribute(Qt::WA_DeleteOnClose);
    m_regWindow->setWindowTitle("Admin Register");

    connect(m_regWindow, &RegisterWidget::destroyed, this, [this]() {
        m_regWindow = nullptr;
        this->show();
    });

    connect(m_regWindow, &RegisterWidget::backToLogin, this, [this]() {
        if (m_regWindow)
            m_regWindow->close();
    });

    connect(m_regWindow, &RegisterWidget::registerSubmitted,
            this, &LoginWidget::registerSubmitted);

    m_regWindow->show();
}

/**
 * @brief Login button click response
 */
void LoginWidget::on_btnLogin_clicked()
{
    QString username = ui->leUsername->text().trimmed();
    QString password = ui->lePassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Tip", "Please enter username and password");
        return;
    }

    emit loginSubmitted(username, password);
}

// ==================== Mouse Events (Window Drag) ====================

/**
 * @brief Mouse press event
 */
void LoginWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        QWidget *child = childAt(pos);

        if (child == ui->label_logo || pos.y() < 80) {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

/**
 * @brief Mouse move event
 */
void LoginWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !m_dragPosition.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

// ==================== Public Slot Functions ====================

/**
 * @brief Show login failure message
 * @param message Error message
 */
void LoginWidget::showLoginError(const QString &message)
{
    QMessageBox::warning(this, "Login Failed", message);
}

/**
 * @brief Show login success message
 * @param message Success message
 */
void LoginWidget::showLoginSuccess(const QString &message)
{
    QMessageBox::information(this, "Login Success", message);
}

/**
 * @brief Navigate to main interface
 * @param userId User ID
 * @param username Username
 * @param role Role (0=User, 1=Admin)
 */
void LoginWidget::navigateToHome(int userId, const QString &username, int role)
{
    this->hide();

    HomeWidget *homeWidget = new HomeWidget(userId, username, role);
    homeWidget->setAttribute(Qt::WA_DeleteOnClose);

    connect(homeWidget, &HomeWidget::destroyed, this, [this]() {
        this->show();
    });

    homeWidget->show();
}

/**
 * @brief Show register error message
 * @param message Error message
 */
void LoginWidget::showRegisterError(const QString &message)
{
    if (m_regWindow)
        m_regWindow->showRegisterError(message);
}

/**
 * @brief Show register success message
 * @param message Success message
 */
void LoginWidget::showRegisterSuccess(const QString &message)
{
    if (m_regWindow)
        m_regWindow->showRegisterSuccess(message);
}

/**
 * @brief Show reset password error message
 * @param message Error message
 */
void LoginWidget::showResetError(const QString &message)
{
    if (m_forgetWindow)
        m_forgetWindow->showResetError(message);
}

/**
 * @brief Show reset password success message
 * @param message Success message
 */
void LoginWidget::showResetSuccess(const QString &message)
{
    if (m_forgetWindow)
        m_forgetWindow->showResetSuccess(message);
}
