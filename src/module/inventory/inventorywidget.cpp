#include "inventorywidget.h"
#include "ui_inventorywidget.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDate>
#include <QDebug>
#include <QFormLayout>

InventoryWidget::InventoryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InventoryWidget)
{
    ui->setupUi(this);
    setupTable();

    // 连接信号槽
    connect(ui->searchButton, &QPushButton::clicked, this, &InventoryWidget::onSearchButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &InventoryWidget::onRefreshButtonClicked);
    connect(ui->addButton, &QPushButton::clicked, this, &InventoryWidget::onAddButtonClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &InventoryWidget::onEditButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &InventoryWidget::onDeleteButtonClicked);
    connect(ui->adjustButton, &QPushButton::clicked, this, &InventoryWidget::onAdjustButtonClicked);
    connect(ui->inventoryTable, &QTableWidget::itemDoubleClicked, this, &InventoryWidget::onTableItemDoubleClicked);

    // 初始加载数据
    refreshInventoryData();
}

InventoryWidget::~InventoryWidget()
{
    delete ui;
}

// ========== 表格初始化 ==========

void InventoryWidget::setupTable()
{
    QStringList headers = {"商品ID", "商品名称", "库存量", "警戒线", "进价", "售价", "最后更新"};
    ui->inventoryTable->setColumnCount(headers.size());
    ui->inventoryTable->setHorizontalHeaderLabels(headers);

    ui->inventoryTable->setColumnWidth(0, 80);   // ID
    ui->inventoryTable->setColumnWidth(1, 150);  // 名称
    ui->inventoryTable->setColumnWidth(2, 80);   // 库存
    ui->inventoryTable->setColumnWidth(3, 80);   // 警戒线
    ui->inventoryTable->setColumnWidth(4, 100);  // 进价
    ui->inventoryTable->setColumnWidth(5, 100);  // 售价
    ui->inventoryTable->setColumnWidth(6, 140);  // 更新日期

    // 设置表格属性
    ui->inventoryTable->setAlternatingRowColors(true);
    ui->inventoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->inventoryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->inventoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void InventoryWidget::clearTable()
{
    ui->inventoryTable->setRowCount(0);
}

// ========== 对外接口实现 ==========

void InventoryWidget::refreshInventoryData()
{
    // TODO: 调用后端接口获取所有商品数据
    // 这里先用模拟数据，后端接入后替换为实际调用

    QList<ProductInfo> products;

    // 模拟数据 - 后端接入后删除这部分
    ProductInfo p1;
    p1.id = 1001;
    p1.name = "笔记本电脑";
    p1.quantity = 15;
    p1.warningLevel = 5;
    p1.purchasePrice = 4500.0;
    p1.salePrice = 4999.0;
    p1.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
    products.append(p1);

    ProductInfo p2;
    p2.id = 1002;
    p2.name = "无线鼠标";
    p2.quantity = 50;
    p2.warningLevel = 10;
    p2.purchasePrice = 25.0;
    p2.salePrice = 39.0;
    p2.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
    products.append(p2);

    ProductInfo p3;
    p3.id = 1003;
    p3.name = "机械键盘";
    p3.quantity = 23;
    p3.warningLevel = 8;
    p3.purchasePrice = 180.0;
    p3.salePrice = 249.0;
    p3.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
    products.append(p3);

    displayProducts(products);
    updateStatus(QString("加载了 %1 条商品记录").arg(products.size()));
}

bool InventoryWidget::addProduct(const ProductInfo &product)
{
    // TODO: 调用后端接口添加商品
    // bool success = InventoryController::instance().addProduct(product);
    bool success = true;  // 模拟成功

    if (success) {
        refreshInventoryData();
        showSuccess("商品添加成功");
    } else {
        showError("商品添加失败");
    }
    return success;
}

bool InventoryWidget::updateProduct(int productId, const ProductInfo &product)
{
    // TODO: 调用后端接口更新商品
    // bool success = InventoryController::instance().updateProduct(productId, product);
    bool success = true;  // 模拟成功

    if (success) {
        refreshInventoryData();
        showSuccess("商品信息更新成功");
    } else {
        showError("商品信息更新失败");
    }
    return success;
}

bool InventoryWidget::deleteProduct(int productId)
{
    // TODO: 调用后端接口删除商品
    // bool success = InventoryController::instance().deleteProduct(productId);
    bool success = true;  // 模拟成功

    if (success) {
        refreshInventoryData();
        showSuccess("商品删除成功");
    } else {
        showError("商品删除失败");
    }
    return success;
}

bool InventoryWidget::updateStock(int productId, int newQuantity)
{
    // TODO: 调用后端接口更新库存
    // bool success = InventoryController::instance().updateStock(productId, newQuantity);
    bool success = true;  // 模拟成功

    if (success) {
        refreshInventoryData();
        showSuccess(QString("库存已更新为 %1").arg(newQuantity));
    } else {
        showError("库存更新失败");
    }
    return success;
}

void InventoryWidget::searchProductByName(const QString &name)
{
    if (name.isEmpty()) {
        refreshInventoryData();
        return;
    }

    // TODO: 调用后端接口搜索商品
    // QList<ProductInfo> results = InventoryController::instance().searchByName(name);

    QList<ProductInfo> results;  // 模拟搜索结果
    for (const ProductInfo &product : m_currentProducts) {
        if (product.name.contains(name, Qt::CaseInsensitive)) {
            results.append(product);
        }
    }

    displayProducts(results);
    updateStatus(QString("找到 %1 条匹配记录").arg(results.size()));
}

int InventoryWidget::getCurrentSelectedProductId() const
{
    int currentRow = ui->inventoryTable->currentRow();
    if (currentRow < 0) {
        return -1;
    }

    QTableWidgetItem *idItem = ui->inventoryTable->item(currentRow, 0);
    if (!idItem) {
        return -1;
    }

    return idItem->text().toInt();
}

QList<ProductInfo> InventoryWidget::getAllProducts() const
{
    return m_currentProducts;
}

// ========== UI 槽函数实现 ==========

void InventoryWidget::onSearchButtonClicked()
{
    QString keyword = ui->searchLineEdit->text().trimmed();
    searchProductByName(keyword);
}

void InventoryWidget::onRefreshButtonClicked()
{
    ui->searchLineEdit->clear();
    refreshInventoryData();
    updateStatus("数据已刷新");
}

void InventoryWidget::onAddButtonClicked()
{
    ProductInfo newProduct;
    newProduct.id = 0;  // 新商品ID由后端生成

    if (showAddProductDialog(newProduct)) {
        addProduct(newProduct);
    }
}

void InventoryWidget::onEditButtonClicked()
{
    int productId = getCurrentSelectedProductId();
    if (productId < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的商品");
        return;
    }

    // TODO: 根据ID获取商品信息
    // ProductInfo product = InventoryController::instance().getProductById(productId);

    // 模拟获取商品信息
    ProductInfo product;
    product.id = productId;
    product.name = "示例商品";
    product.quantity = 10;
    product.warningLevel = 5;
    product.purchasePrice = 100.0;
    product.salePrice = 150.0;
    product.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");

    if (showEditProductDialog(product)) {
        updateProduct(productId, product);
    }
}

void InventoryWidget::onDeleteButtonClicked()
{
    int productId = getCurrentSelectedProductId();
    if (productId < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的商品");
        return;
    }

    // 获取商品名称用于确认对话框
    int currentRow = ui->inventoryTable->currentRow();
    QString productName = ui->inventoryTable->item(currentRow, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除商品 \"%1\" 吗？\n此操作不可恢复！").arg(productName),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        deleteProduct(productId);
    }
}

void InventoryWidget::onAdjustButtonClicked()
{
    int productId = getCurrentSelectedProductId();
    if (productId < 0) {
        QMessageBox::warning(this, "提示", "请先选择要盘点的商品");
        return;
    }

    // 获取当前库存
    int currentRow = ui->inventoryTable->currentRow();
    int currentQuantity = ui->inventoryTable->item(currentRow, 2)->text().toInt();
    QString productName = ui->inventoryTable->item(currentRow, 1)->text();

    int newQuantity;
    if (showStockAdjustDialog(productId, currentQuantity, newQuantity)) {
        updateStock(productId, newQuantity);
    }
}

void InventoryWidget::onTableItemDoubleClicked(QTableWidgetItem *item)
{
    Q_UNUSED(item);
    onEditButtonClicked();
}

// ========== 内部辅助函数 ==========

void InventoryWidget::displayProducts(const QList<ProductInfo> &products)
{
    clearTable();
    m_currentProducts = products;

    for (int i = 0; i < products.size(); ++i) {
        const ProductInfo &product = products[i];
        ui->inventoryTable->insertRow(i);
        addRowToTable(product, i);

        // 如果库存低于警戒线，标记为红色
        if (product.quantity <= product.warningLevel) {
            for (int col = 0; col < ui->inventoryTable->columnCount(); ++col) {
                QTableWidgetItem *item = ui->inventoryTable->item(i, col);
                if (item) {
                    item->setForeground(Qt::red);
                }
            }
        }
    }
}

void InventoryWidget::addRowToTable(const ProductInfo &product, int row)
{
    ui->inventoryTable->setItem(row, 0, new QTableWidgetItem(QString::number(product.id)));
    ui->inventoryTable->setItem(row, 1, new QTableWidgetItem(product.name));
    ui->inventoryTable->setItem(row, 2, new QTableWidgetItem(QString::number(product.quantity)));
    ui->inventoryTable->setItem(row, 3, new QTableWidgetItem(QString::number(product.warningLevel)));
    ui->inventoryTable->setItem(row, 4, new QTableWidgetItem(QString::number(product.purchasePrice, 'f', 2)));
    ui->inventoryTable->setItem(row, 5, new QTableWidgetItem(QString::number(product.salePrice, 'f', 2)));
    ui->inventoryTable->setItem(row, 6, new QTableWidgetItem(product.lastUpdate));
}

ProductInfo InventoryWidget::getProductFromCurrentRow() const
{
    ProductInfo product;
    int row = ui->inventoryTable->currentRow();
    if (row >= 0) {
        product.id = ui->inventoryTable->item(row, 0)->text().toInt();
        product.name = ui->inventoryTable->item(row, 1)->text();
        product.quantity = ui->inventoryTable->item(row, 2)->text().toInt();
        product.warningLevel = ui->inventoryTable->item(row, 3)->text().toInt();
        product.purchasePrice = ui->inventoryTable->item(row, 4)->text().toDouble();
        product.salePrice = ui->inventoryTable->item(row, 5)->text().toDouble();
        product.lastUpdate = ui->inventoryTable->item(row, 6)->text();
    }
    return product;
}

void InventoryWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void InventoryWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void InventoryWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}

// ========== 对话框实现 ==========

bool InventoryWidget::showAddProductDialog(ProductInfo &product)
{
    QDialog dialog(this);
    dialog.setWindowTitle("新增商品");
    dialog.setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *nameEdit = new QLineEdit();
    QLineEdit *quantityEdit = new QLineEdit();
    QLineEdit *warningEdit = new QLineEdit();
    QLineEdit *purchaseEdit = new QLineEdit();
    QLineEdit *saleEdit = new QLineEdit();

    quantityEdit->setValidator(new QIntValidator(0, 99999, quantityEdit));
    warningEdit->setValidator(new QIntValidator(0, 99999, warningEdit));
    purchaseEdit->setValidator(new QDoubleValidator(0, 999999, 2, purchaseEdit));
    saleEdit->setValidator(new QDoubleValidator(0, 999999, 2, saleEdit));

    formLayout->addRow("商品名称:", nameEdit);
    formLayout->addRow("初始库存:", quantityEdit);
    formLayout->addRow("警戒线:", warningEdit);
    formLayout->addRow("进价:", purchaseEdit);
    formLayout->addRow("售价:", saleEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(const_cast<QWidget*>(static_cast<const QWidget*>(this)), "提示", "商品名称不能为空");
            return false;
        }

        product.name = nameEdit->text();
        product.quantity = quantityEdit->text().toInt();
        product.warningLevel = warningEdit->text().toInt();
        product.purchasePrice = purchaseEdit->text().toDouble();
        product.salePrice = saleEdit->text().toDouble();
        product.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
        return true;
    }

    return false;
}

bool InventoryWidget::showEditProductDialog(ProductInfo &product)
{
    QDialog dialog(this);
    dialog.setWindowTitle("编辑商品");
    dialog.setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *nameEdit = new QLineEdit(product.name);
    QLineEdit *quantityEdit = new QLineEdit(QString::number(product.quantity));
    QLineEdit *warningEdit = new QLineEdit(QString::number(product.warningLevel));
    QLineEdit *purchaseEdit = new QLineEdit(QString::number(product.purchasePrice));
    QLineEdit *saleEdit = new QLineEdit(QString::number(product.salePrice));

    quantityEdit->setValidator(new QIntValidator(0, 99999, quantityEdit));
    warningEdit->setValidator(new QIntValidator(0, 99999, warningEdit));
    purchaseEdit->setValidator(new QDoubleValidator(0, 999999, 2, purchaseEdit));
    saleEdit->setValidator(new QDoubleValidator(0, 999999, 2, saleEdit));

    formLayout->addRow("商品ID:", new QLabel(QString::number(product.id)));
    formLayout->addRow("商品名称:", nameEdit);
    formLayout->addRow("库存量:", quantityEdit);
    formLayout->addRow("警戒线:", warningEdit);
    formLayout->addRow("进价:", purchaseEdit);
    formLayout->addRow("售价:", saleEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(const_cast<QWidget*>(static_cast<const QWidget*>(this)), "提示", "商品名称不能为空");
            return false;
        }

        product.name = nameEdit->text();
        product.quantity = quantityEdit->text().toInt();
        product.warningLevel = warningEdit->text().toInt();
        product.purchasePrice = purchaseEdit->text().toDouble();
        product.salePrice = saleEdit->text().toDouble();
        product.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
        return true;
    }

    return false;
}

bool InventoryWidget::showStockAdjustDialog(int productId, int currentQuantity, int &newQuantity)
{
    QDialog dialog(this);
    dialog.setWindowTitle("库存盘点");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    formLayout->addRow("商品ID:", new QLabel(QString::number(productId)));
    formLayout->addRow("当前库存:", new QLabel(QString::number(currentQuantity)));

    QLineEdit *newQuantityEdit = new QLineEdit();
    newQuantityEdit->setValidator(new QIntValidator(0, 99999, newQuantityEdit));
    newQuantityEdit->setPlaceholderText("请输入盘点后的实际数量");
    formLayout->addRow("盘点后库存:", newQuantityEdit);

    QLabel *tipLabel = new QLabel("提示：\n• 盘盈：新库存 > 当前库存\n• 盘亏：新库存 < 当前库存\n• 将自动记录盘点差异");
    tipLabel->setStyleSheet("color: gray; font-size: 11px;");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(tipLabel);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        newQuantity = newQuantityEdit->text().toInt();

        int diff = newQuantity - currentQuantity;
        if (diff > 0) {
            QMessageBox::information(&dialog, "盘盈", QString("盘盈数量: +%1").arg(diff));
        } else if (diff < 0) {
            QMessageBox::information(&dialog, "盘亏", QString("盘亏数量: %1").arg(diff));
        } else {
            QMessageBox::information(&dialog, "无变化", "库存数量未发生变化");
        }

        return true;
    }

    return false;
}