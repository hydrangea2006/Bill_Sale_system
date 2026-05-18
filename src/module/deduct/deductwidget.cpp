#include "deductwidget.h"
#include "ui_deductwidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QDateTime>

DeductWidget::DeductWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeductWidget)
{
    ui->setupUi(this);
    setupTable();

    // 连接信号槽
    connect(ui->confirmButton, &QPushButton::clicked, this, &DeductWidget::onConfirmButtonClicked);
    connect(ui->resetButton, &QPushButton::clicked, this, &DeductWidget::onResetButtonClicked);
    connect(ui->searchProductButton, &QPushButton::clicked, this, &DeductWidget::onSearchProductButtonClicked);
    connect(ui->refreshHistoryButton, &QPushButton::clicked, this, &DeductWidget::onRefreshHistoryButtonClicked);
    connect(ui->productCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DeductWidget::onProductComboChanged);
    connect(ui->quantitySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &DeductWidget::onQuantityOrPriceChanged);
    connect(ui->priceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DeductWidget::onQuantityOrPriceChanged);
    connect(ui->historyTable, &QTableWidget::itemDoubleClicked, this, &DeductWidget::onHistoryTableDoubleClicked);

    // 初始化数据
    refreshProductList();
    refreshHistoryRecords();

    updateStatus("就绪");
}

DeductWidget::~DeductWidget()
{
    delete ui;
}

// ========== 表格初始化 ==========

void DeductWidget::setupTable()
{
    QStringList headers = {"记录ID", "商品ID", "商品名称", "出库数量", "销售单价", "总金额", "客户", "出库时间"};
    ui->historyTable->setColumnCount(headers.size());
    ui->historyTable->setHorizontalHeaderLabels(headers);

    ui->historyTable->setColumnWidth(0, 60);
    ui->historyTable->setColumnWidth(1, 60);
    ui->historyTable->setColumnWidth(2, 150);
    ui->historyTable->setColumnWidth(3, 80);
    ui->historyTable->setColumnWidth(4, 100);
    ui->historyTable->setColumnWidth(5, 100);
    ui->historyTable->setColumnWidth(6, 120);
    ui->historyTable->setColumnWidth(7, 140);

    ui->historyTable->setAlternatingRowColors(true);
    ui->historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->historyTable->setSelectionMode(QAbstractItemView::SingleSelection);
}

// ========== 对外接口实现（待后端接入） ==========

void DeductWidget::refreshProductList()
{
    // TODO: 从库存模块获取商品列表
    ui->productCombo->clear();
    ui->productCombo->addItem("请选择商品", -1);
    ui->productCombo->addItem("笔记本电脑", 1001);
    ui->productCombo->addItem("无线鼠标", 1002);
    ui->productCombo->addItem("机械键盘", 1003);

    updateStatus(QString("加载了 %1 个商品").arg(ui->productCombo->count() - 1));
}

void DeductWidget::refreshHistoryRecords()
{
    // TODO: 从数据库获取出库记录
    QList<DeductRecord> records;

    // 模拟数据
    DeductRecord r1;
    r1.id = 1;
    r1.productId = 1001;
    r1.productName = "笔记本电脑";
    r1.quantity = 2;
    r1.price = 4999.00;
    r1.total = 9998.00;
    r1.customer = "张三";
    r1.remark = "";
    r1.createTime = QDate::currentDate().toString("yyyy-MM-dd");
    records.append(r1);

    DeductRecord r2;
    r2.id = 2;
    r2.productId = 1002;
    r2.productName = "无线鼠标";
    r2.quantity = 10;
    r2.price = 39.00;
    r2.total = 390.00;
    r2.customer = "李四";
    r2.remark = "";
    r2.createTime = QDate::currentDate().toString("yyyy-MM-dd");
    records.append(r2);

    displayRecords(records);
    updateStatus(QString("加载了 %1 条出库记录").arg(records.size()));
}

bool DeductWidget::addDeductRecord(const DeductRecord &record)
{
    Q_UNUSED(record);
    // TODO: 调用后端接口保存出库记录
    return true;
}

QList<DeductRecord> DeductWidget::getRecentRecords(int limit)
{
    Q_UNUSED(limit);
    // TODO: 从数据库获取
    return m_currentRecords;
}

int DeductWidget::getProductStock(int productId)
{
    Q_UNUSED(productId);
    // TODO: 从库存模块获取
    return 50;  // 模拟返回
}

double DeductWidget::getProductSalePrice(int productId)
{
    Q_UNUSED(productId);
    // TODO: 从库存模块获取
    return 99.00;  // 模拟返回
}

void DeductWidget::clearForm()
{
    ui->productCombo->setCurrentIndex(0);
    ui->quantitySpin->setValue(1);
    ui->priceSpin->setValue(0.00);
    ui->customerEdit->clear();
    ui->remarkEdit->clear();
    ui->currentStockValue->setText("-- 件");
    updateTotalDisplay();
}

// ========== UI 槽函数实现 ==========

void DeductWidget::onConfirmButtonClicked()
{
    if (!validateForm()) {
        return;
    }

    int productId = getCurrentProductId();
    QString productName = ui->productCombo->currentText();
    int quantity = ui->quantitySpin->value();
    double price = ui->priceSpin->value();
    double total = quantity * price;
    QString customer = ui->customerEdit->text().trimmed();
    QString remark = ui->remarkEdit->toPlainText().trimmed();

    // 确认对话框
    QString msg = QString("确认出库以下商品？\n\n"
                          "商品：%1\n"
                          "数量：%2 件\n"
                          "单价：¥ %3\n"
                          "总金额：¥ %4\n"
                          "客户：%5")
                      .arg(productName)
                      .arg(quantity)
                      .arg(price, 0, 'f', 2)
                      .arg(total, 0, 'f', 2)
                      .arg(customer.isEmpty() ? "无" : customer);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认出库", msg, QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 检查库存是否足够
    if (!checkStockEnough(productId, quantity)) {
        showError(QString("库存不足！当前库存只有 %1 件").arg(getProductStock(productId)));
        return;
    }

    // 创建出库记录
    DeductRecord record;
    record.productId = productId;
    record.productName = productName;
    record.quantity = quantity;
    record.price = price;
    record.total = total;
    record.customer = customer;
    record.remark = remark;
    record.createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 保存记录
    if (saveDeductRecord(record)) {
        // 更新库存（扣减）
        if (performStockOut(productId, quantity)) {
            showSuccess(QString("出库成功！%1 出库 %2 件，总金额 ¥ %3")
                            .arg(productName).arg(quantity).arg(total, 0, 'f', 2));
            clearForm();
            refreshHistoryRecords();
            refreshProductList();
        }
    }
}

void DeductWidget::onResetButtonClicked()
{
    clearForm();
    updateStatus("表单已重置");
}

void DeductWidget::onSearchProductButtonClicked()
{
    QMessageBox::information(this, "搜索商品", "此处可搜索商品");
}

void DeductWidget::onRefreshHistoryButtonClicked()
{
    refreshHistoryRecords();
}

void DeductWidget::onProductComboChanged(int index)
{
    if (index > 0) {
        int productId = ui->productCombo->currentData().toInt();
        QString productName = ui->productCombo->currentText();

        int stock = getProductStock(productId);
        double salePrice = getProductSalePrice(productId);

        ui->currentStockValue->setText(QString("%1 件").arg(stock));
        ui->priceSpin->setValue(salePrice);

        updateStatus(QString("已选择商品：%1，当前库存：%2 件").arg(productName).arg(stock));
    } else {
        ui->currentStockValue->setText("-- 件");
        ui->priceSpin->setValue(0.00);
    }
}

void DeductWidget::onQuantityOrPriceChanged()
{
    updateTotalDisplay();
}

void DeductWidget::onHistoryTableDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        int row = item->row();
        int recordId = ui->historyTable->item(row, 0)->text().toInt();
        updateStatus(QString("查看出库记录 #%1").arg(recordId));
    }
}

// ========== 内部辅助函数 ==========

void DeductWidget::displayRecords(const QList<DeductRecord> &records)
{
    ui->historyTable->setRowCount(0);
    m_currentRecords = records;

    for (int i = 0; i < records.size(); ++i) {
        ui->historyTable->insertRow(i);
        addRecordToTable(records[i], i);
    }
}

void DeductWidget::addRecordToTable(const DeductRecord &record, int row)
{
    ui->historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.id)));
    ui->historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.productId)));
    ui->historyTable->setItem(row, 2, new QTableWidgetItem(record.productName));
    ui->historyTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.quantity)));
    ui->historyTable->setItem(row, 4, new QTableWidgetItem(QString::number(record.price, 'f', 2)));
    ui->historyTable->setItem(row, 5, new QTableWidgetItem(QString::number(record.total, 'f', 2)));
    ui->historyTable->setItem(row, 6, new QTableWidgetItem(record.customer));
    ui->historyTable->setItem(row, 7, new QTableWidgetItem(record.createTime));
}

void DeductWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void DeductWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void DeductWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}

void DeductWidget::updateTotalDisplay()
{
    int quantity = ui->quantitySpin->value();
    double price = ui->priceSpin->value();
    double total = quantity * price;
    ui->totalLabel->setText(QString("合计：¥ %1").arg(total, 0, 'f', 2));
}

void DeductWidget::updateCurrentStockDisplay()
{
    int productId = getCurrentProductId();
    if (productId > 0) {
        int stock = getProductStock(productId);
        ui->currentStockValue->setText(QString("%1 件").arg(stock));
    }
}

bool DeductWidget::performStockOut(int productId, int quantity)
{
    Q_UNUSED(productId);
    Q_UNUSED(quantity);
    // TODO: 调用库存模块，扣减库存
    return true;
}

bool DeductWidget::saveDeductRecord(const DeductRecord &record)
{
    Q_UNUSED(record);
    // TODO: 保存到数据库
    return true;
}

int DeductWidget::getCurrentProductId() const
{
    return ui->productCombo->currentData().toInt();
}

bool DeductWidget::validateForm()
{
    if (ui->productCombo->currentIndex() <= 0) {
        QMessageBox::warning(this, "提示", "请选择商品");
        return false;
    }

    if (ui->quantitySpin->value() <= 0) {
        QMessageBox::warning(this, "提示", "请输入有效的出库数量");
        return false;
    }

    if (ui->priceSpin->value() <= 0) {
        QMessageBox::warning(this, "提示", "请输入有效的销售单价");
        return false;
    }

    return true;
}

bool DeductWidget::checkStockEnough(int productId, int quantity)
{
    int currentStock = getProductStock(productId);
    return currentStock >= quantity;
}