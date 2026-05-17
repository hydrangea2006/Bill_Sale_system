#ifndef CARTWIDGET_H
#define CARTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class CartWidget; }
QT_END_NAMESPACE

// 入库记录数据结构
struct CartRecord {
    int id;                 // 记录ID
    int productId;          // 商品ID
    QString productName;    // 商品名称
    int quantity;           // 入库数量
    double price;           // 入库单价
    double total;           // 总金额
    QString supplier;       // 供应商
    QString remark;         // 备注
    QString createTime;     // 入库时间
};

class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(QWidget *parent = nullptr);
    ~CartWidget();

    // ========== 对外接口（供后端/controller调用） ==========

    // 刷新商品下拉列表（从库存模块获取）
    void refreshProductList();

    // 刷新入库记录表格
    void refreshHistoryRecords();

    // 添加入库记录（后端调用）
    bool addCartRecord(const CartRecord &record);

    // 获取最近的入库记录
    QList<CartRecord> getRecentRecords(int limit = 20);

    // 根据商品ID获取商品当前库存
    int getProductStock(int productId);

    // 清空表单
    void clearForm();

private slots:
    // ========== UI 槽函数 ==========
    void onConfirmButtonClicked();
    void onResetButtonClicked();
    void onSearchProductButtonClicked();
    void onRefreshHistoryButtonClicked();
    void onProductComboChanged(int index);
    void onQuantityOrPriceChanged();  // 数量或单价改变时更新小计
    void onHistoryTableDoubleClicked(QTableWidgetItem *item);

private:
    // ========== 内部辅助函数 ==========
    void setupTable();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void updateTotalDisplay();

    // 表格操作
    void displayRecords(const QList<CartRecord> &records);
    void addRecordToTable(const CartRecord &record, int row);

    // 入库操作
    bool performStockIn(int productId, int quantity);
    bool saveCartRecord(const CartRecord &record);

    // 获取选中的商品ID
    int getCurrentProductId() const;

    // 验证表单
    bool validateForm();

private:
    Ui::CartWidget *ui;
    QList<CartRecord> m_currentRecords;  // 当前显示的入库记录
};

#endif // CARTWIDGET_H