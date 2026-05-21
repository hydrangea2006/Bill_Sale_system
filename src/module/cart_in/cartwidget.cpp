#include "cartwidget.h"
#include "ui_cartwidget.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QDebug>

CartWidget::CartWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWidget)
    , m_currentUserId(0)
    , m_currentBalance(0.0)
{
    ui->setupUi(this);
    setupTables();

    connect(ui->productCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CartWidget::onProductComboChanged);
    connect(ui->addToCartButton, &QPushButton::clicked,
            this, &CartWidget::onAddToCartButtonClicked);
    connect(ui->clearCartButton, &QPushButton::clicked,
            this, &CartWidget::onClearCartButtonClicked);
    connect(ui->checkoutButton, &QPushButton::clicked,
            this, &CartWidget::onCheckoutButtonClicked);
    connect(ui->refreshOrdersButton, &QPushButton::clicked,
            this, &CartWidget::onRefreshOrdersButtonClicked);
    connect(ui->cartTable, &QTableWidget::itemDoubleClicked,
            this, &CartWidget::onCartTableDoubleClicked);

    double initBalance = DatabaseManager::instance().getBalance();
    setCurrentUser(1, "测试用户", initBalance);
    updateStatus("就绪");
}

CartWidget::~CartWidget()
{
    delete ui;
}

void CartWidget::setupTables()
{
    QStringList cartHeaders = {"商品ID", "商品名称", "数量", "单价", "小计", "操作"};
    ui->cartTable->setColumnCount(cartHeaders.size());
    ui->cartTable->setHorizontalHeaderLabels(cartHeaders);
    ui->cartTable->setColumnWidth(0, 60);
    ui->cartTable->setColumnWidth(1, 200);
    ui->cartTable->setColumnWidth(2, 80);
    ui->cartTable->setColumnWidth(3, 100);
    ui->cartTable->setColumnWidth(4, 100);
    ui->cartTable->setColumnWidth(5, 100);

    QStringList orderHeaders = {"订单号", "总金额", "状态", "下单时间"};
    ui->ordersTable->setColumnCount(orderHeaders.size());
    ui->ordersTable->setHorizontalHeaderLabels(orderHeaders);
    ui->ordersTable->setColumnWidth(0, 150);
    ui->ordersTable->setColumnWidth(1, 100);
    ui->ordersTable->setColumnWidth(2, 100);
    ui->ordersTable->setColumnWidth(3, 150);

    ui->cartTable->setAlternatingRowColors(true);
    ui->cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->ordersTable->setAlternatingRowColors(true);
    ui->ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void CartWidget::setCurrentUser(int userId, const QString &username, double balance)
{
    m_currentUserId = userId;
    m_currentUsername = username;
    m_currentBalance = balance;
    ui->usernameLabel->setText(username);
    refreshBalance();
    refreshProductList();
    refreshCart();
    refreshOrders();
}

void CartWidget::refreshProductList()
{
    ui->productCombo->clear();
    ui->productCombo->addItem("请选择商品", -1);

    DatabaseManager& db = DatabaseManager::instance();
    QList<ProductRecord> products = db.getAllProducts();
    for (const auto& p : products) {
        if (p.quantity > 0) {
            ui->productCombo->addItem(
                QString("%1 (¥%2)").arg(p.name).arg(p.salePrice, 0, 'f', 2),
                p.id
            );
        }
    }
}

void CartWidget::refreshCart()
{
    displayCartItems(m_cartItems);
    updateTotalDisplay();
}

void CartWidget::refreshOrders()
{
    QList<Order> orders;

    DatabaseManager& db = DatabaseManager::instance();
    QList<SalesOrderRecord> dbOrders = db.getUserOrders(m_currentUserId);

    for (const auto& o : dbOrders) {
        Order order;
        order.id = o.id;
        order.orderNo = QString("%1%2").arg(o.createdAt.left(8)).arg(o.id, 4, 10, QChar('0'));
        order.totalAmount = o.totalAmount;
        order.status = "已支付";
        order.createTime = o.createdAt;
        orders.append(order);
    }

    m_orders = orders;
    displayOrders(orders);
}

void CartWidget::refreshBalance()
{
    ui->balanceLabel->setText(QString("余额：¥ %1").arg(m_currentBalance, 0, 'f', 2));
}

bool CartWidget::addToCart(int productId, int quantity)
{
    int stock = getProductStock(productId);
    if (quantity > stock) {
        showError(QString("库存不足！当前库存：%1 件").arg(stock));
        return false;
    }

    for (CartItem &item : m_cartItems) {
        if (item.productId == productId) {
            int newQuantity = item.quantity + quantity;
            if (newQuantity > stock) {
                showError(QString("加入后数量超过库存！当前库存：%1 件").arg(stock));
                return false;
            }
            item.quantity = newQuantity;
            item.total = item.quantity * item.price;
            refreshCart();
            showSuccess(QString("已更新 %1 数量为 %2").arg(item.productName).arg(newQuantity));
            return true;
        }
    }

    CartItem newItem;
    newItem.productId = productId;
    newItem.productName = ui->productCombo->currentText();
    newItem.quantity = quantity;
    newItem.price = getProductPrice(productId);
    newItem.total = quantity * newItem.price;
    m_cartItems.append(newItem);
    refreshCart();
    showSuccess(QString("已添加 %1 x %2 件").arg(newItem.productName).arg(quantity));
    return true;
}

bool CartWidget::removeFromCart(int productId)
{
    for (int i = 0; i < m_cartItems.size(); ++i) {
        if (m_cartItems[i].productId == productId) {
            m_cartItems.removeAt(i);
            refreshCart();
            return true;
        }
    }
    return false;
}

bool CartWidget::updateCartQuantity(int productId, int quantity)
{
    for (CartItem &item : m_cartItems) {
        if (item.productId == productId) {
            if (quantity <= 0) {
                return removeFromCart(productId);
            }
            int stock = getProductStock(productId);
            if (quantity > stock) {
                showError(QString("库存不足！当前库存：%1 件").arg(stock));
                return false;
            }
            item.quantity = quantity;
            item.total = quantity * item.price;
            refreshCart();
            return true;
        }
    }
    return false;
}

bool CartWidget::clearCart()
{
    if (m_cartItems.isEmpty()) {
        showError("购物车已经是空的");
        return false;
    }
    m_cartItems.clear();
    refreshCart();
    updateStatus("购物车已清空");
    return true;
}

QList<CartItem> CartWidget::getCartItems()
{
    return m_cartItems;
}

double CartWidget::getCartTotal()
{
    double total = 0;
    for (const CartItem &item : m_cartItems) {
        total += item.total;
    }
    return total;
}

bool CartWidget::checkout()
{
    if (m_cartItems.isEmpty()) {
        showError("购物车为空，请先添加商品");
        return false;
    }

    double total = getCartTotal();

    if (total > m_currentBalance) {
        showError(QString("余额不足！需要 ¥ %1，当前余额 ¥ %2")
                      .arg(total, 0, 'f', 2)
                      .arg(m_currentBalance, 0, 'f', 2));
        return false;
    }

    for (const CartItem &item : m_cartItems) {
        int stock = getProductStock(item.productId);
        if (item.quantity > stock) {
            showError(QString("商品 %1 库存不足！当前库存：%2 件")
                          .arg(item.productName).arg(stock));
            return false;
        }
    }

    QString msg = QString("确认下单？\n\n订单总额：¥ %1\n支付后余额：¥ %2")
                      .arg(total, 0, 'f', 2)
                      .arg(m_currentBalance - total, 0, 'f', 2);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认下单", msg, QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return false;
    }

    QList<SalesOrderItemRecord> items;
    for (const auto& ci : m_cartItems) {
        SalesOrderItemRecord item;
        item.productId = ci.productId;
        item.quantity = ci.quantity;
        item.unitPrice = ci.price;
        item.subtotal = ci.total;
        items.append(item);
    }

    DatabaseManager& db = DatabaseManager::instance();
    int orderId = db.createOrder(m_currentUserId, "", items);
    if (orderId <= 0) {
        showError("下单失败");
        return false;
    }

    m_currentBalance = db.getBalance();
    m_cartItems.clear();
    refreshCart();
    refreshOrders();
    refreshBalance();

    showSuccess(QString("下单成功！订单号：%1").arg(orderId));
    return true;
}

QList<Order> CartWidget::getUserOrders()
{
    return m_orders;
}

void CartWidget::onProductComboChanged(int index)
{
    if (index > 0) {
        int productId = ui->productCombo->currentData().toInt();
        int stock = getProductStock(productId);
        double price = getProductPrice(productId);
        ui->stockLabel->setText(QString("库存：%1 件").arg(stock));
        ui->priceLabel->setText(QString("单价：¥ %1").arg(price, 0, 'f', 2));
        ui->quantitySpin->setMaximum(stock);
    } else {
        ui->stockLabel->setText("库存：-- 件");
        ui->priceLabel->setText("单价：¥ --");
        ui->quantitySpin->setMaximum(999);
    }
}

void CartWidget::onAddToCartButtonClicked()
{
    if (ui->productCombo->currentIndex() <= 0) {
        showError("请选择商品");
        return;
    }
    int productId = ui->productCombo->currentData().toInt();
    int quantity = ui->quantitySpin->value();
    addToCart(productId, quantity);
}

void CartWidget::onClearCartButtonClicked()
{
    if (m_cartItems.isEmpty()) {
        showError("购物车已经是空的");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认清空", "确定要清空购物车吗？",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        clearCart();
    }
}

void CartWidget::onCheckoutButtonClicked()
{
    checkout();
}

void CartWidget::onRefreshOrdersButtonClicked()
{
    refreshOrders();
    updateStatus("订单列表已刷新");
}

void CartWidget::onCartTableDoubleClicked(QTableWidgetItem *item)
{
    if (!item) return;
    int row = item->row();
    if (row < 0 || row >= m_cartItems.size()) return;
    int productId = m_cartItems[row].productId;
    int currentQuantity = m_cartItems[row].quantity;
    bool ok;
    int newQuantity = QInputDialog::getInt(this, "修改数量", "请输入新数量：", currentQuantity, 1, 999, 1, &ok);
    if (ok) {
        updateCartQuantity(productId, newQuantity);
    }
}

void CartWidget::displayCartItems(const QList<CartItem> &items)
{
    ui->cartTable->setRowCount(0);
    for (int i = 0; i < items.size(); ++i) {
        ui->cartTable->insertRow(i);
        addCartItemToTable(items[i], i);
        QPushButton *deleteBtn = new QPushButton("删除");
        ui->cartTable->setCellWidget(i, 5, deleteBtn);
        connect(deleteBtn, &QPushButton::clicked, this, [this, productId = items[i].productId]() {
            removeFromCart(productId);
        });
    }
}

void CartWidget::addCartItemToTable(const CartItem &item, int row)
{
    ui->cartTable->setItem(row, 0, new QTableWidgetItem(QString::number(item.productId)));
    ui->cartTable->setItem(row, 1, new QTableWidgetItem(item.productName));
    ui->cartTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
    ui->cartTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.price, 'f', 2)));
    ui->cartTable->setItem(row, 4, new QTableWidgetItem(QString::number(item.total, 'f', 2)));
}

void CartWidget::displayOrders(const QList<Order> &orders)
{
    ui->ordersTable->setRowCount(0);
    for (int i = 0; i < orders.size(); ++i) {
        ui->ordersTable->insertRow(i);
        addOrderToTable(orders[i], i);
    }
}

void CartWidget::addOrderToTable(const Order &order, int row)
{
    ui->ordersTable->setItem(row, 0, new QTableWidgetItem(order.orderNo));
    ui->ordersTable->setItem(row, 1, new QTableWidgetItem(QString::number(order.totalAmount, 'f', 2)));
    ui->ordersTable->setItem(row, 2, new QTableWidgetItem(order.status));
    ui->ordersTable->setItem(row, 3, new QTableWidgetItem(order.createTime));
}

void CartWidget::updateTotalDisplay()
{
    double total = getCartTotal();
    ui->totalLabel->setText(QString("总计：¥ %1").arg(total, 0, 'f', 2));
}

void CartWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void CartWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void CartWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}

int CartWidget::getProductStock(int productId)
{
    ProductRecord p = DatabaseManager::instance().getProductById(productId);
    return (p.id > 0) ? p.quantity : 0;
}

double CartWidget::getProductPrice(int productId)
{
    ProductRecord p = DatabaseManager::instance().getProductById(productId);
    return (p.id > 0) ? p.salePrice : 0.0;
}

bool CartWidget::deductBalance(int userId, double amount)
{
    Q_UNUSED(userId);
    return DatabaseManager::instance().deductBalance(amount);
}

bool CartWidget::updateProductStock(int productId, int quantity)
{
    return DatabaseManager::instance().deductProductStock(productId, quantity);
}