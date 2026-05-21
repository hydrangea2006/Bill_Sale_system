#include "balancewidget.h"
#include "ui_balancewidget.h"
#include "databasemanager.h"
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

    DatabaseManager& db = DatabaseManager::instance();
    QList<DatabaseManager::TransactionRecord> dbRecords = db.getAllTransactions();

    // 从最新一条倒推每笔流水发生时的余额
    double runningBalance = db.getBalance();
    for (int i = 0; i < dbRecords.size(); ++i) {
        const auto& dr = dbRecords[i];
        TransactionRecord rec;
        rec.id = dr.id;
        rec.type = (dr.type == 1) ? "充值" : "消费";
        rec.amount = dr.amount;
        // 先记录当前余额（即该笔交易发生后的余额）
        rec.balance = runningBalance;
        // 再倒推：收入就减回去，支出就加回去
        if (dr.type == 1) {
            runningBalance -= dr.amount;
        } else {
            runningBalance += dr.amount;
        }
        rec.remark = dr.remark;
        rec.createTime = dr.createdAt;
        records.prepend(rec);  // prepend 使最终按时间正序排列
    }

    displayTransactions(records);
    updateStatus(QString("加载了 %1 条交易记录").arg(records.size()));
}

bool BalanceWidget::recharge(double amount)
{
    DatabaseManager& db = DatabaseManager::instance();
    if (!db.rechargeBalance(amount)) {
        showError("充值失败");
        return false;
    }
    db.addTransaction(1, amount, QString("充值 ¥%1").arg(amount, 0, 'f', 2));
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
    DatabaseManager& db = DatabaseManager::instance();
    if (!db.deductBalance(amount)) {
        showError("提现失败");
        return false;
    }
    db.addTransaction(0, amount, QString("提现 ¥%1").arg(amount, 0, 'f', 2));
    refreshBalance();
    refreshTransactionHistory();
    return true;
}

double BalanceWidget::getCurrentBalance()
{
    return DatabaseManager::instance().getBalance();
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