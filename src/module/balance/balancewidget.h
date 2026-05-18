#ifndef BALANCEWIDGET_H
#define BALANCEWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class BalanceWidget; }
QT_END_NAMESPACE

struct TransactionRecord {
    int id;
    QString type;
    double amount;
    double balance;
    QString remark;
    QString createTime;
};

class BalanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BalanceWidget(QWidget *parent = nullptr);
    ~BalanceWidget();

    void refreshBalance();
    void refreshTransactionHistory();
    bool recharge(double amount);
    bool withdraw(double amount);
    double getCurrentBalance();

private slots:
    void onRechargeButtonClicked();
    void onWithdrawButtonClicked();
    void onRefreshButtonClicked();

private:
    void setupTable();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void displayTransactions(const QList<TransactionRecord> &records);
    void addTransactionToTable(const TransactionRecord &record, int row);

private:
    Ui::BalanceWidget *ui;
    QList<TransactionRecord> m_currentTransactions;
};

#endif // BALANCEWIDGET_H