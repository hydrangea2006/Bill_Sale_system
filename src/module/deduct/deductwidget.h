#ifndef DEDUCTWIDGET_H
#define DEDUCTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class DeductWidget; }
QT_END_NAMESPACE

// 出库记录数据结构
struct DeductRecord {
    int id;                 // 记录ID
    int productId;          // 商品ID
    QString productName;    // 商品名称
    int quantity;           // 出库数量
    double price;           // 销售单价
    double total;           // 总金额
    QString customer;       // 客户
    QString remark;         // 备注
    QString createTime;     // 出库时间
};

class DeductWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeductWidget(QWidget *parent = nullptr);
    ~DeductWidget();

    // ========== 对外接口（供后端/controller调用） ==========

    // 刷新商品下拉列表（从库存模块获取）
    void refreshProductList();

    // 刷新出库记录表格
    void refreshHistoryRecords();

    // 添加出库记录（后端调用）
    bool addDeductRecord(const DeductRecord &record);

    // 获取最近的出库记录
    QList<DeductRecord> getRecentRecords(int limit = 20);

    // 根据商品ID获取商品当前库存和售价
    int getProductStock(int productId);
    double getProductSalePrice(int productId);

    // 清空表单
    void clearForm();

private slots:
    // ========== UI 槽函数 ==========
    void onConfirmButtonClicked();
    void onResetButtonClicked();
    void onSearchProductButtonClicked();
    void onRefreshHistoryButtonClicked();
    void onProductComboChanged(int index);
    void onQuantityOrPriceChanged();
    void onHistoryTableDoubleClicked(QTableWidgetItem *item);

private:
    // ========== 内部辅助函数 ==========
    void setupTable();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void updateTotalDisplay();
    void updateCurrentStockDisplay();

    // 表格操作
    void displayRecords(const QList<DeductRecord> &records);
    void addRecordToTable(const DeductRecord &record, int row);

    // 出库操作
    bool performStockOut(int productId, int quantity);
    bool saveDeductRecord(const DeductRecord &record);

    // 获取选中的商品ID
    int getCurrentProductId() const;

    // 验证表单
    bool validateForm();
    bool checkStockEnough(int productId, int quantity);

private:
    Ui::DeductWidget *ui;
    QList<DeductRecord> m_currentRecords;  // 当前显示的出库记录
};

#endif // DEDUCTWIDGET_H