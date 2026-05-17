#include "balancewidget.h"
#include "ui_balancewidget.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QDateTime>

BalanceWidget::BalanceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BalanceWidget)
{
    ui->setupUi(this);
    setupTable();

    connect(ui->rechargeButton, &QPushButton::clicked, this, &BalanceWidget::onRechargeButtonClicked);
    connect(ui->withdrawButton, &QPushButton::clicked, this, &BalanceWidget::onWithdrawButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &BalanceWidget::onRefreshButtonClicked);

    refreshBalance();
    refreshTransactionHistory();
    updateStatus("就绪");
}

BalanceWidget::~BalanceWidget()
{
    delete ui;
}

void BalanceWidget::setupTable()
{
    QStringList headers = {"记录ID", "类型", "金额", "余额", "备注", "时间"};
    ui->historyTable->setColumnCount(headers.size());
    ui->historyTable->setHorizontalHeaderLabels(headers);

    ui->historyTable->setColumnWidth(0, 60);
    ui->historyTable->setColumnWidth(1, 80);
    ui->historyTable->setColumnWidth(2, 100);
    ui->historyTable->setColumnWidth(3, 100);
    ui->historyTable->setColumnWidth(4, 200);
    ui->historyTable->setColumnWidth(5, 150);

    ui->historyTable->setAlternatingRowColors(true);
    ui->historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void BalanceWidget::refreshBalance()
{
    double balance = getCurrentBalance();
    ui->balanceAmountLabel->setText(QString("¥ %1").arg(balance, 0, 'f', 2));
}

void BalanceWidget::refreshTransactionHistory()
{
    QList<TransactionRecord> records;

    TransactionRecord r1;
    r1.id = 1;
    r1.type = "充值";
    r1.amount = 500.00;
    r1.balance = 500.00;
    r1.remark = "微信充值";
    r1.createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    records.append(r1);

    TransactionRecord r2;
    r2.id = 2;
    r2.type = "消费";
    r2.amount = 39.00;
    r2.balance = 461.00;
    r2.remark = "购买无线鼠标";
    r2.createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    records.append(r2);

    displayTransactions(records);
    updateStatus(QString("加载了 %1 条交易记录").arg(records.size()));
}

bool BalanceWidget::recharge(double amount)
{
    Q_UNUSED(amount);
    refreshBalance();
    refreshTransactionHistory();
    return true;
}

bool BalanceWidget::withdraw(double amount)
{
    double current = getCurrentBalance();
    if (amount > current) {
        showError("余额不足，无法提现");
        return false;
    }
    refreshBalance();
    refreshTransactionHistory();
    return true;
}

double BalanceWidget::getCurrentBalance()
{
    return 500.00;
}

void BalanceWidget::onRechargeButtonClicked()
{
    bool ok;
    double amount = QInputDialog::getDouble(this, "充值", "请输入充值金额：", 0, 0, 999999, 2, &ok);
    if (ok && amount > 0) {
        if (recharge(amount)) {
            showSuccess(QString("充值成功！充值金额：¥ %1").arg(amount, 0, 'f', 2));
        }
    }
}

void BalanceWidget::onWithdrawButtonClicked()
{
    bool ok;
    double amount = QInputDialog::getDouble(this, "提现", "请输入提现金额：", 0, 0, getCurrentBalance(), 2, &ok);
    if (ok && amount > 0) {
        if (withdraw(amount)) {
            showSuccess(QString("提现成功！提现金额：¥ %1").arg(amount, 0, 'f', 2));
        }
    }
}

void BalanceWidget::onRefreshButtonClicked()
{
    refreshBalance();
    refreshTransactionHistory();
    updateStatus("数据已刷新");
}

void BalanceWidget::displayTransactions(const QList<TransactionRecord> &records)
{
    ui->historyTable->setRowCount(0);
    m_currentTransactions = records;

    for (int i = 0; i < records.size(); ++i) {
        ui->historyTable->insertRow(i);
        addTransactionToTable(records[i], i);
    }
}

void BalanceWidget::addTransactionToTable(const TransactionRecord &record, int row)
{
    ui->historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.id)));
    ui->historyTable->setItem(row, 1, new QTableWidgetItem(record.type));
    ui->historyTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.amount, 'f', 2)));
    ui->historyTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.balance, 'f', 2)));
    ui->historyTable->setItem(row, 4, new QTableWidgetItem(record.remark));
    ui->historyTable->setItem(row, 5, new QTableWidgetItem(record.createTime));
}

void BalanceWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void BalanceWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void BalanceWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}