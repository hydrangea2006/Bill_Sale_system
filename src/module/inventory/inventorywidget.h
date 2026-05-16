#ifndef INVENTORYWIDGET_H
#define INVENTORYWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class InventoryWidget; }
QT_END_NAMESPACE

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(QWidget *parent = nullptr);
    ~InventoryWidget();

private slots:
    void onSearchButtonClicked();
    void onRefreshButtonClicked();
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onAdjustButtonClicked();
    void onTableItemDoubleClicked(QTableWidgetItem *item);

private:
    void setupTable();
    void loadInventoryData();
    void updateStatus(const QString &message);

private:
    Ui::InventoryWidget *ui;
};

#endif // INVENTORYWIDGET_H