#include "addresswidget.h"
#include "ui_addresswidget.h"
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>

AddressWidget::AddressWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddressWidget)
{
    ui->setupUi(this);
    setupTable();

    connect(ui->addButton, &QPushButton::clicked, this, &AddressWidget::onAddButtonClicked);

    refreshAddressList();
    updateStatus("就绪");
}

AddressWidget::~AddressWidget()
{
    delete ui;
}

void AddressWidget::setupTable()
{
    QStringList headers = {"ID", "收货人", "电话", "地址", "默认", "操作"};
    ui->addressTable->setColumnCount(headers.size());
    ui->addressTable->setHorizontalHeaderLabels(headers);

    ui->addressTable->setColumnWidth(0, 50);
    ui->addressTable->setColumnWidth(1, 100);
    ui->addressTable->setColumnWidth(2, 120);
    ui->addressTable->setColumnWidth(3, 350);
    ui->addressTable->setColumnWidth(4, 60);
    ui->addressTable->setColumnWidth(5, 150);

    ui->addressTable->setAlternatingRowColors(true);
    ui->addressTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void AddressWidget::refreshAddressList()
{
    QList<AddressInfo> addresses;

    AddressInfo a1;
    a1.id = 1;
    a1.name = "张三";
    a1.phone = "13800138001";
    a1.province = "广东省";
    a1.city = "深圳市";
    a1.district = "南山区";
    a1.detail = "科技园路100号";
    a1.isDefault = true;
    addresses.append(a1);

    AddressInfo a2;
    a2.id = 2;
    a2.name = "李四";
    a2.phone = "13800138002";
    a2.province = "广东省";
    a2.city = "广州市";
    a2.district = "天河区";
    a2.detail = "体育西路200号";
    a2.isDefault = false;
    addresses.append(a2);

    displayAddresses(addresses);
    updateStatus(QString("加载了 %1 个地址").arg(addresses.size()));
}

bool AddressWidget::addAddress(const AddressInfo &address)
{
    Q_UNUSED(address);
    refreshAddressList();
    return true;
}

bool AddressWidget::updateAddress(int id, const AddressInfo &address)
{
    Q_UNUSED(id);
    Q_UNUSED(address);
    refreshAddressList();
    return true;
}

bool AddressWidget::deleteAddress(int id)
{
    Q_UNUSED(id);
    refreshAddressList();
    return true;
}

bool AddressWidget::setDefaultAddress(int id)
{
    Q_UNUSED(id);
    refreshAddressList();
    return true;
}

void AddressWidget::onAddButtonClicked()
{
    AddressInfo newAddress;
    if (showAddressDialog(newAddress, false)) {
        addAddress(newAddress);
        showSuccess("地址添加成功");
    }
}

void AddressWidget::onEditButtonClicked()
{
    int row = ui->addressTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的地址");
        return;
    }

    AddressInfo address = m_currentAddresses[row];
    if (showAddressDialog(address, true)) {
        updateAddress(address.id, address);
        showSuccess("地址更新成功");
    }
}

void AddressWidget::onDeleteButtonClicked()
{
    int row = ui->addressTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的地址");
        return;
    }

    QString name = ui->addressTable->item(row, 1)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除", QString("确定要删除地址 \"%1\" 吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int id = ui->addressTable->item(row, 0)->text().toInt();
        deleteAddress(id);
        showSuccess("地址删除成功");
    }
}

void AddressWidget::onSetDefaultButtonClicked()
{
    int row = ui->addressTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要设为默认的地址");
        return;
    }

    int id = ui->addressTable->item(row, 0)->text().toInt();
    setDefaultAddress(id);
    showSuccess("已设为默认地址");
}

void AddressWidget::onTableDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        onEditButtonClicked();
    }
}

void AddressWidget::displayAddresses(const QList<AddressInfo> &addresses)
{
    ui->addressTable->setRowCount(0);
    m_currentAddresses = addresses;

    for (int i = 0; i < addresses.size(); ++i) {
        ui->addressTable->insertRow(i);
        addAddressToTable(addresses[i], i);

        QPushButton *editBtn = new QPushButton("编辑");
        QPushButton *deleteBtn = new QPushButton("删除");
        QPushButton *defaultBtn = new QPushButton("设为默认");

        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->addWidget(editBtn);
        layout->addWidget(deleteBtn);
        if (!addresses[i].isDefault) {
            layout->addWidget(defaultBtn);
        }
        layout->setContentsMargins(5, 2, 5, 2);
        layout->setSpacing(5);
        widget->setLayout(layout);

        ui->addressTable->setCellWidget(i, 5, widget);

        connect(editBtn, &QPushButton::clicked, this, &AddressWidget::onEditButtonClicked);
        connect(deleteBtn, &QPushButton::clicked, this, &AddressWidget::onDeleteButtonClicked);
        connect(defaultBtn, &QPushButton::clicked, this, &AddressWidget::onSetDefaultButtonClicked);
    }
}

void AddressWidget::addAddressToTable(const AddressInfo &address, int row)
{
    QString fullAddress = QString("%1 %2 %3 %4").arg(address.province, address.city, address.district, address.detail);
    ui->addressTable->setItem(row, 0, new QTableWidgetItem(QString::number(address.id)));
    ui->addressTable->setItem(row, 1, new QTableWidgetItem(address.name));
    ui->addressTable->setItem(row, 2, new QTableWidgetItem(address.phone));
    ui->addressTable->setItem(row, 3, new QTableWidgetItem(fullAddress));
    ui->addressTable->setItem(row, 4, new QTableWidgetItem(address.isDefault ? "是" : ""));
}

bool AddressWidget::showAddressDialog(AddressInfo &address, bool isEdit)
{
    QDialog dialog(this);
    dialog.setWindowTitle(isEdit ? "编辑地址" : "新增地址");
    dialog.setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *nameEdit = new QLineEdit(address.name);
    QLineEdit *phoneEdit = new QLineEdit(address.phone);
    QComboBox *provinceCombo = new QComboBox();
    QComboBox *cityCombo = new QComboBox();
    QComboBox *districtCombo = new QComboBox();
    QLineEdit *detailEdit = new QLineEdit(address.detail);
    QCheckBox *defaultCheck = new QCheckBox("设为默认地址");

    provinceCombo->addItems({"广东省", "北京市", "上海市"});
    cityCombo->addItems({"深圳市", "广州市"});
    districtCombo->addItems({"南山区", "福田区", "罗湖区"});

    provinceCombo->setCurrentText(address.province);
    cityCombo->setCurrentText(address.city);
    districtCombo->setCurrentText(address.district);
    defaultCheck->setChecked(address.isDefault);

    formLayout->addRow("收货人：", nameEdit);
    formLayout->addRow("电话：", phoneEdit);
    formLayout->addRow("省份：", provinceCombo);
    formLayout->addRow("城市：", cityCombo);
    formLayout->addRow("区/县：", districtCombo);
    formLayout->addRow("详细地址：", detailEdit);
    formLayout->addRow("", defaultCheck);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        address.name = nameEdit->text();
        address.phone = phoneEdit->text();
        address.province = provinceCombo->currentText();
        address.city = cityCombo->currentText();
        address.district = districtCombo->currentText();
        address.detail = detailEdit->text();
        address.isDefault = defaultCheck->isChecked();
        return true;
    }
    return false;
}

void AddressWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void AddressWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void AddressWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}