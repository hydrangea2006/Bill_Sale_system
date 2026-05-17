#ifndef INVENTORYWIDGET_H
#define INVENTORYWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QList>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class InventoryWidget; }
QT_END_NAMESPACE

// 商品数据结构
struct ProductInfo {
    int id;
    QString name;
    int quantity;
    int warningLevel;
    double purchasePrice;
    double salePrice;
    QString lastUpdate;
};

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(QWidget *parent = nullptr);
    ~InventoryWidget();

    // ========== 对外接口（供后端/controller调用） ==========

    // 刷新/加载所有商品数据
    void refreshInventoryData();

    // 添加商品（后端调用此接口添加数据）
    bool addProduct(const ProductInfo &product);

    // 更新商品信息
    bool updateProduct(int productId, const ProductInfo &product);

    // 删除商品
    bool deleteProduct(int productId);

    // 更新库存数量（入库/出库时调用）
    bool updateStock(int productId, int newQuantity);

    // 根据商品名称搜索
    void searchProductByName(const QString &name);

    // 获取当前选中的商品ID
    int getCurrentSelectedProductId() const;

    // 获取所有商品列表
    QList<ProductInfo> getAllProducts() const;

private slots:
    // ========== UI 槽函数 ==========
    void onSearchButtonClicked();
    void onRefreshButtonClicked();
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onAdjustButtonClicked();
    void onTableItemDoubleClicked(QTableWidgetItem *item);

private:
    // ========== 内部辅助函数 ==========
    void setupTable();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);

    // 表格操作
    void displayProducts(const QList<ProductInfo> &products);
    void addRowToTable(const ProductInfo &product, int row);
    ProductInfo getProductFromCurrentRow() const;

    // 对话框
    bool showAddProductDialog(ProductInfo &product);
    bool showEditProductDialog(ProductInfo &product);
    bool showStockAdjustDialog(int productId, int currentQuantity, int &newQuantity);

    // 清空表格
    void clearTable();

private:
    Ui::InventoryWidget *ui;
    QList<ProductInfo> m_currentProducts;  // 当前显示的商品列表
};

#endif // INVENTORYWIDGET_H