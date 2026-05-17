#include "cartwidget.h"
#include "ui_cartwidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>

CartWidget::CartWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWidget)
{
    ui->setupUi(this);
    setupTable();

    // 连接信号槽
    connect(ui->confirmButton, &QPushButton::clicked, this, &CartWidget::onConfirmButtonClicked);
    connect(ui->resetButton, &QPushButton::clicked, this, &CartWidget::onResetButtonClicked);
    connect(ui->searchProductButton, &QPushButton::clicked, this, &CartWidget::onSearchProductButtonClicked);
    connect(ui->refreshHistoryButton, &QPushButton::clicked, this, &CartWidget::onRefreshHistoryButtonClicked);
    connect(ui->productCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartWidget::onProductComboChanged);
    connect(ui->quantitySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartWidget::onQuantityOrPriceChanged);
    connect(ui->priceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartWidget::onQuantityOrPriceChanged);
    connect(ui->historyTable, &QTableWidget::itemDoubleClicked, this, &CartWidget::onHistoryTableDoubleClicked);

    // 初始化数据
    refreshProductList();
    refreshHistoryRecords();

    updateStatus("就绪");
}

CartWidget::~CartWidget()
{
    delete ui;
}

// ========== 表格初始化 ==========

void CartWidget::setupTable()
{
    QStringList headers = {"记录ID", "商品ID", "商品名称", "入库数量", "入库单价", "总金额", "供应商", "入库时间"};
    ui->historyTable->setColumnCount(headers.size());
    ui->historyTable->setHorizontalHeaderLabels(headers);

    ui->historyTable->setColumnWidth(0, 60);   // 记录ID
    ui->historyTable->setColumnWidth(1, 60);   // 商品ID
    ui->historyTable->setColumnWidth(2, 150);  // 商品名称
    ui->historyTable->setColumnWidth(3, 80);   // 入库数量
    ui->historyTable->setColumnWidth(4, 100);  // 入库单价
    ui->historyTable->setColumnWidth(5, 100);  // 总金额
    ui->historyTable->setColumnWidth(6, 120);  // 供应商
    ui->historyTable->setColumnWidth(7, 140);  // 入库时间

    ui->historyTable->setAlternatingRowColors(true);
    ui->historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->historyTable->setSelectionMode(QAbstractItemView::SingleSelection);
}

// ========== 对外接口实现 ==========

void CartWidget::refreshProductList()
{
    // TODO: 从库存模块获取商品列表
    // QList<ProductInfo> products = InventoryController::instance().getAllProducts();

    ui->productCombo->clear();

    // 模拟数据 - 后端接入后删除
    ui->productCombo->addItem("请选择商品", -1);
    ui->productCombo->addItem("笔记本电脑", 1001);
    ui->productCombo->addItem("无线鼠标", 1002);
    ui->productCombo->addItem("机械键盘", 1003);

    updateStatus(QString("加载了 %1 个商品").arg(ui->productCombo->count() - 1));
}

void CartWidget::refreshHistoryRecords()
{
    // TODO: 从数据库获取入库记录
    // QList<CartRecord> records = CartController::instance().getRecentRecords(20);

    QList<CartRecord> records;  // 模拟数据

    // 模拟记录
    CartRecord r1;
    r1.id = 1;
    r1.productId = 1001;
    r1.productName = "笔记本电脑";
    r1.quantity = 10;
    r1.price = 4500.00;
    r1.total = 45000.00;
    r1.supplier = "联想科技";
    r1.remark = "首批进货";
    r1.createTime = QDate::currentDate().toString("yyyy-MM-dd");
    records.append(r1);

    CartRecord r2;
    r2.id = 2;
    r2.productId = 1002;
    r2.productName = "无线鼠标";
    r2.quantity = 50;
    r2.price = 25.00;
    r2.total = 1250.00;
    r2.supplier = "罗技电子";
    r2.remark = "";
    r2.createTime = QDate::currentDate().toString("yyyy-MM-dd");
    records.append(r2);

    displayRecords(records);
    updateStatus(QString("加载了 %1 条入库记录").arg(records.size()));
}

bool CartWidget::addCartRecord(const CartRecord &record)
{
    // TODO: 调用后端接口保存入库记录
    // bool success = CartController::instance().addRecord(record);

    bool success = true;  // 模拟成功

    if (success) {
        refreshHistoryRecords();
        showSuccess("入库成功");
    } else {
        showError("入库失败");
    }
    return success;
}

QList<CartRecord> CartWidget::getRecentRecords(int limit)
{
    Q_UNUSED(limit);
    // TODO: 从数据库获取
    return m_currentRecords;
}

int CartWidget::getProductStock(int productId)
{
    Q_UNUSED(productId);
    // TODO: 从库存模块获取商品当前库存
    return 0;
}

void CartWidget::clearForm()
{
    ui->productCombo->setCurrentIndex(0);
    ui->quantitySpin->setValue(1);
    ui->priceSpin->setValue(0.00);
    ui->supplierEdit->clear();
    ui->remarkEdit->clear();
    updateTotalDisplay();
}

// ========== UI 槽函数实现 ==========

void CartWidget::onConfirmButtonClicked()
{
    if (!validateForm()) {
        return;
    }

    int productId = getCurrentProductId();
    QString productName = ui->productCombo->currentText();
    int quantity = ui->quantitySpin->value();
    double price = ui->priceSpin->value();
    double total = quantity * price;
    QString supplier = ui->supplierEdit->text().trimmed();
    QString remark = ui->remarkEdit->toPlainText().trimmed();

    // 确认对话框
    QString msg = QString("确认入库以下商品？\n\n"
                          "商品：%1\n"
                          "数量：%2 件\n"
                          "单价：¥ %3\n"
                          "总金额：¥ %4\n"
                          "供应商：%5")
                      .arg(productName)
                      .arg(quantity)
                      .arg(price, 0, 'f', 2)
                      .arg(total, 0, 'f', 2)
                      .arg(supplier.isEmpty() ? "无" : supplier);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认入库", msg, QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 创建入库记录
    CartRecord record;
    record.productId = productId;
    record.productName = productName;
    record.quantity = quantity;
    record.price = price;
    record.total = total;
    record.supplier = supplier;
    record.remark = remark;
    record.createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 保存记录
    if (saveCartRecord(record)) {
        // 更新库存
        if (performStockIn(productId, quantity)) {
            showSuccess(QString("入库成功！%1 入库 %2 件，总金额 ¥ %3")
                            .arg(productName).arg(quantity).arg(total, 0, 'f', 2));
            clearForm();
            refreshHistoryRecords();
            refreshProductList();  // 刷新商品列表（库存量会变）
        }
    }
}

void CartWidget::onResetButtonClicked()
{
    clearForm();
    updateStatus("表单已重置");
}

void CartWidget::onSearchProductButtonClicked()
{
    // TODO: 打开商品搜索/新增对话框
    QMessageBox::information(this, "搜索商品", "此处可搜索商品或新增商品");
}

void CartWidget::onRefreshHistoryButtonClicked()
{
    refreshHistoryRecords();
}

void CartWidget::onProductComboChanged(int index)
{
    if (index > 0) {
        int productId = ui->productCombo->currentData().toInt();
        QString productName = ui->productCombo->currentText();

        // TODO: 获取商品当前库存和售价
        // int stock = getProductStock(productId);
        // double salePrice = getProductSalePrice(productId);

        updateStatus(QString("已选择商品：%1").arg(productName));

        // 可选：自动填充建议单价
        // ui->priceSpin->setValue(salePrice);
    }
}

void CartWidget::onQuantityOrPriceChanged()
{
    updateTotalDisplay();
}

void CartWidget::onHistoryTableDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        int row = item->row();
        int recordId = ui->historyTable->item(row, 0)->text().toInt();
        updateStatus(QString("查看入库记录 #%1").arg(recordId));
        // TODO: 显示记录详情对话框
    }
}

// ========== 内部辅助函数 ==========

void CartWidget::displayRecords(const QList<CartRecord> &records)
{
    ui->historyTable->setRowCount(0);
    m_currentRecords = records;

    for (int i = 0; i < records.size(); ++i) {
        ui->historyTable->insertRow(i);
        addRecordToTable(records[i], i);
    }
}

void CartWidget::addRecordToTable(const CartRecord &record, int row)
{
    ui->historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.id)));
    ui->historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.productId)));
    ui->historyTable->setItem(row, 2, new QTableWidgetItem(record.productName));
    ui->historyTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.quantity)));
    ui->historyTable->setItem(row, 4, new QTableWidgetItem(QString::number(record.price, 'f', 2)));
    ui->historyTable->setItem(row, 5, new QTableWidgetItem(QString::number(record.total, 'f', 2)));
    ui->historyTable->setItem(row, 6, new QTableWidgetItem(record.supplier));
    ui->historyTable->setItem(row, 7, new QTableWidgetItem(record.createTime));
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

void CartWidget::updateTotalDisplay()
{
    int quantity = ui->quantitySpin->value();
    double price = ui->priceSpin->value();
    double total = quantity * price;
    ui->totalLabel->setText(QString("小计：¥ %1").arg(total, 0, 'f', 2));
}

bool CartWidget::performStockIn(int productId, int quantity)
{
    Q_UNUSED(productId);
    Q_UNUSED(quantity);
    // TODO: 调用库存模块，增加库存
    // bool success = InventoryController::instance().addStock(productId, quantity);
    return true;  // 模拟成功
}

bool CartWidget::saveCartRecord(const CartRecord &record)
{
    Q_UNUSED(record);
    // TODO: 保存到数据库
    // QSqlQuery query;
    // query.prepare("INSERT INTO cart_in_records (product_id, product_name, quantity, price, total, supplier, remark, create_time) "
    //               "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    // ...
    return true;  // 模拟成功
}

int CartWidget::getCurrentProductId() const
{
    return ui->productCombo->currentData().toInt();
}

bool CartWidget::validateForm()
{
    if (ui->productCombo->currentIndex() <= 0) {
        QMessageBox::warning(this, "提示", "请选择商品");
        return false;
    }

    if (ui->quantitySpin->value() <= 0) {
        QMessageBox::warning(this, "提示", "请输入有效的入库数量");
        return false;
    }

    if (ui->priceSpin->value() <= 0) {
        QMessageBox::warning(this, "提示", "请输入有效的入库单价");
        return false;
    }

    return true;
}