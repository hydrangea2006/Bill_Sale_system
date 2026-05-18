#ifndef ADDRESSWIDGET_H
#define ADDRESSWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class AddressWidget; }
QT_END_NAMESPACE

struct AddressInfo {
    int id;
    QString name;
    QString phone;
    QString province;
    QString city;
    QString district;
    QString detail;
    bool isDefault;
};

class AddressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddressWidget(QWidget *parent = nullptr);
    ~AddressWidget();

    void refreshAddressList();
    bool addAddress(const AddressInfo &address);
    bool updateAddress(int id, const AddressInfo &address);
    bool deleteAddress(int id);
    bool setDefaultAddress(int id);

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onSetDefaultButtonClicked();
    void onTableDoubleClicked(QTableWidgetItem *item);

private:
    void setupTable();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void displayAddresses(const QList<AddressInfo> &addresses);
    void addAddressToTable(const AddressInfo &address, int row);
    bool showAddressDialog(AddressInfo &address, bool isEdit = false);

private:
    Ui::AddressWidget *ui;
    QList<AddressInfo> m_currentAddresses;
};

#endif // ADDRESSWIDGET_H