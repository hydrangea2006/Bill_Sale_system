#include "inventorywidget.h"
#include "ui_inventorywidget.h"
#include <QMessageBox>
#include <QDebug>

InventoryWidget::InventoryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InventoryWidget)
{
    ui->setupUi(this);
    setupTable();
    loadInventoryData();

    // 连接信号槽
    connect(ui->searchButton, &QPushButton::clicked, this, &InventoryWidget::onSearchButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &InventoryWidget::onRefreshButtonClicked);
    connect(ui->addButton, &QPushButton::clicked, this, &InventoryWidget::onAddButtonClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &InventoryWidget::onEditButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &InventoryWidget::onDeleteButtonClicked);
    connect(ui->adjustButton, &QPushButton::clicked, this, &InventoryWidget::onAdjustButtonClicked);
    connect(ui->inventoryTable, &QTableWidget::itemDoubleClicked, this, &InventoryWidget::onTableItemDoubleClicked);
}

InventoryWidget::~InventoryWidget()
{
    delete ui;
}

void InventoryWidget::setupTable()
{
    QStringList headers = {"商品ID", "商品名称", "库存量", "警戒线", "进价", "售价", "最后更新"};
    ui->inventoryTable->setColumnCount(headers.size());
    ui->inventoryTable->setHorizontalHeaderLabels(headers);

    ui->inventoryTable->setColumnWidth(0, 80);
    ui->inventoryTable->setColumnWidth(1, 150);
    ui->inventoryTable->setColumnWidth(2, 80);
    ui->inventoryTable->setColumnWidth(3, 80);
    ui->inventoryTable->setColumnWidth(4, 100);
    ui->inventoryTable->setColumnWidth(5, 100);
    ui->inventoryTable->setColumnWidth(6, 140);
}

void InventoryWidget::loadInventoryData()
{
    ui->inventoryTable->setRowCount(0);

    // 示例数据
    QStringList sampleData[] = {
        {"1001", "笔记本电脑", "15", "5", "4500", "4999", "2024-01-15"},
        {"1002", "无线鼠标", "50", "10", "25", "39", "2024-01-16"},
        {"1003", "机械键盘", "23", "8", "180", "249", "2024-01-16"}
    };

    for (int row = 0; row < 3; ++row) {
        ui->inventoryTable->insertRow(row);
        for (int col = 0; col < sampleData[row].size(); ++col) {
            ui->inventoryTable->setItem(row, col, new QTableWidgetItem(sampleData[row][col]));
        }
    }

    updateStatus(QString("加载了 %1 条库存记录").arg(3));
}

void InventoryWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void InventoryWidget::onSearchButtonClicked()
{
    QString keyword = ui->searchLineEdit->text();
    updateStatus(QString("正在搜索: %1").arg(keyword));
}

void InventoryWidget::onRefreshButtonClicked()
{
    loadInventoryData();
    updateStatus("数据已刷新");
}

void InventoryWidget::onAddButtonClicked()
{
    QMessageBox::information(this, "新增商品", "打开新增商品窗口");
}

void InventoryWidget::onEditButtonClicked()
{
    int row = ui->inventoryTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的商品");
        return;
    }
    QString productId = ui->inventoryTable->item(row, 0)->text();
    QMessageBox::information(this, "编辑商品", QString("编辑商品: %1").arg(productId));
}

void InventoryWidget::onDeleteButtonClicked()
{
    int row = ui->inventoryTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的商品");
        return;
    }

    QString productName = ui->inventoryTable->item(row, 1)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除商品 \"%1\" 吗？").arg(productName),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        ui->inventoryTable->removeRow(row);
        updateStatus(QString("已删除商品: %1").arg(productName));
    }
}

void InventoryWidget::onAdjustButtonClicked()
{
    int row = ui->inventoryTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要盘点的商品");
        return;
    }
    QString productName = ui->inventoryTable->item(row, 1)->text();
    QMessageBox::information(this, "库存盘点", QString("盘点商品: %1").arg(productName));
}

void InventoryWidget::onTableItemDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        onEditButtonClicked();
    }
}