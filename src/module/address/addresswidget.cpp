#include "addresswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>

AddressWidget::AddressWidget(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    setupUI();
    emit refreshRequested();
}

AddressWidget::~AddressWidget()
{
}

void AddressWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel* titleLabel = new QLabel("Address Management", this);
    titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #303133; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 提示信息
    QLabel* tipLabel = new QLabel("Manage your shipping addresses for quick selection during checkout.", this);
    tipLabel->setStyleSheet("QLabel { color: #909399; font-size: 12px; margin-bottom: 10px; }");
    tipLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(tipLabel);

    // 顶部按钮栏
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_addBtn = new QPushButton("➕ Add Address", this);
    m_editBtn = new QPushButton("✏️ Edit Address", this);
    m_deleteBtn = new QPushButton("🗑️ Delete Address", this);
    m_setDefaultBtn = new QPushButton("⭐ Set Default", this);
    m_refreshBtn = new QPushButton("🔄 Refresh", this);

    m_addBtn->setFixedSize(120, 36);
    m_editBtn->setFixedSize(120, 36);
    m_deleteBtn->setFixedSize(120, 36);
    m_setDefaultBtn->setFixedSize(120, 36);
    m_refreshBtn->setFixedSize(80, 36);

    m_addBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 13px; }");
    m_editBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; font-size: 13px; }");
    m_deleteBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 4px; font-size: 13px; }");
    m_setDefaultBtn->setStyleSheet("QPushButton { background-color: #E6A23C; color: white; border-radius: 4px; font-size: 13px; }");
    m_refreshBtn->setStyleSheet("QPushButton { background-color: #909399; color: white; border-radius: 4px; font-size: 13px; }");

    topLayout->addWidget(m_addBtn);
    topLayout->addWidget(m_editBtn);
    topLayout->addWidget(m_deleteBtn);
    topLayout->addWidget(m_setDefaultBtn);
    topLayout->addStretch();
    topLayout->addWidget(m_refreshBtn);

    mainLayout->addLayout(topLayout);

    // 地址表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(7);
    QStringList headers = {"ID", "Consignee", "Phone", "Region", "Detailed Address", "Default", "Action"};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setMinimumHeight(400);

    // 设置列宽
    m_tableWidget->setColumnWidth(0, 50);
    m_tableWidget->setColumnWidth(1, 120);
    m_tableWidget->setColumnWidth(2, 120);
    m_tableWidget->setColumnWidth(3, 200);
    m_tableWidget->setColumnWidth(4, 250);
    m_tableWidget->setColumnWidth(5, 60);
    m_tableWidget->setColumnWidth(6, 100);

    mainLayout->addWidget(m_tableWidget);

    // 连接信号
    connect(m_addBtn, &QPushButton::clicked, this, &AddressWidget::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &AddressWidget::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &AddressWidget::onDeleteClicked);
    connect(m_setDefaultBtn, &QPushButton::clicked, this, &AddressWidget::onSetDefaultClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &AddressWidget::onRefreshClicked);
}

void AddressWidget::showAddressDialog(int addressId, const QVariantMap& data)
{
    QDialog dialog(this);
    dialog.setWindowTitle(addressId < 0 ? "Add Address" : "Edit Address");
    dialog.setFixedSize(450, 550);
    dialog.setModal(true);

    QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setSpacing(15);
    dialogLayout->setContentsMargins(20, 20, 20, 20);

    QFormLayout* form = new QFormLayout();
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    // 收货人
    QLineEdit* nameEdit = new QLineEdit(&dialog);
    nameEdit->setPlaceholderText("Please enter consignee name");
    nameEdit->setMinimumHeight(36);
    if (data.contains("name")) nameEdit->setText(data["name"].toString());
    form->addRow("Consignee:", nameEdit);

    // 手机号
    QLineEdit* phoneEdit = new QLineEdit(&dialog);
    phoneEdit->setPlaceholderText("Please enter phone number");
    phoneEdit->setMinimumHeight(36);
    if (data.contains("phone")) phoneEdit->setText(data["phone"].toString());
    form->addRow("Phone Number:", phoneEdit);

    // 省份
    QComboBox* provinceCombo = new QComboBox(&dialog);
    provinceCombo->setEditable(true);
    provinceCombo->addItems({"Beijing", "Shanghai", "Guangdong", "Jiangsu", "Zhejiang", "Sichuan", "Hubei", "Hunan", "Fujian", "Shandong", "Henan", "Hebei", "Anhui", "Shaanxi", "Chongqing"});
    provinceCombo->setMinimumHeight(36);
    if (data.contains("province")) provinceCombo->setCurrentText(data["province"].toString());
    form->addRow("Province:", provinceCombo);

    // 城市
    QComboBox* cityCombo = new QComboBox(&dialog);
    cityCombo->setEditable(true);
    cityCombo->setMinimumHeight(36);
    if (data.contains("city")) cityCombo->setCurrentText(data["city"].toString());
    form->addRow("City:", cityCombo);

    // 区/县
    QComboBox* districtCombo = new QComboBox(&dialog);
    districtCombo->setEditable(true);
    districtCombo->setMinimumHeight(36);
    if (data.contains("district")) districtCombo->setCurrentText(data["district"].toString());
    form->addRow("District:", districtCombo);

    // 详细地址
    QTextEdit* detailEdit = new QTextEdit(&dialog);
    detailEdit->setPlaceholderText("Please enter detailed address (street, community, house number)");
    detailEdit->setFixedHeight(80);
    if (data.contains("detail")) detailEdit->setPlainText(data["detail"].toString());
    form->addRow("Detailed Address:", detailEdit);

    // 设为默认
    QComboBox* isDefaultCombo = new QComboBox(&dialog);
    isDefaultCombo->addItems({"No", "Yes"});
    isDefaultCombo->setMinimumHeight(36);
    if (data.contains("isDefault") && data["isDefault"].toBool()) {
        isDefaultCombo->setCurrentIndex(1);
    }
    form->addRow("Set as Default:", isDefaultCombo);

    dialogLayout->addLayout(form);

    // 按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);
    QPushButton* submitBtn = new QPushButton(addressId < 0 ? "Confirm Add" : "Confirm Edit", &dialog);
    QPushButton* cancelBtn = new QPushButton("Cancel", &dialog);
    submitBtn->setFixedSize(120, 40);
    cancelBtn->setFixedSize(80, 40);
    submitBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #909399; color: white; border-radius: 4px; font-size: 14px; }");
    btnLayout->addStretch();
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();
    dialogLayout->addLayout(btnLayout);

    connect(submitBtn, &QPushButton::clicked, [&]() {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Tip", "Please enter consignee name");
            return;
        }
        if (phoneEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Tip", "Please enter phone number");
            return;
        }
        QString phone = phoneEdit->text();
        if (phone.length() != 11 || !phone.contains(QRegularExpression("^1[3-9]\\d{9}$"))) {
            QMessageBox::warning(&dialog, "Tip", "Please enter a valid phone number");
            return;
        }
        if (detailEdit->toPlainText().isEmpty()) {
            QMessageBox::warning(&dialog, "Tip", "Please enter detailed address");
            return;
        }

        QVariantMap address;
        address["name"] = nameEdit->text();
        address["phone"] = phoneEdit->text();
        address["province"] = provinceCombo->currentText();
        address["city"] = cityCombo->currentText();
        address["district"] = districtCombo->currentText();
        address["detail"] = detailEdit->toPlainText();
        address["isDefault"] = (isDefaultCombo->currentIndex() == 1);
        address["userId"] = m_userId;

        if (addressId < 0) {
            emit addAddressRequested(address);
        } else {
            emit updateAddressRequested(addressId, address);
        }
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void AddressWidget::onAddClicked()
{
    showAddressDialog();
}

void AddressWidget::onEditClicked()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Tip", "Please select an address to edit first");
        return;
    }

    int addressId = m_tableWidget->item(currentRow, 0)->text().toInt();
    QVariantMap data;
    data["name"] = m_tableWidget->item(currentRow, 1)->text();
    data["phone"] = m_tableWidget->item(currentRow, 2)->text();

    QString location = m_tableWidget->item(currentRow, 3)->text();
    QStringList parts = location.split(" / ");
    data["province"] = parts.size() > 0 ? parts[0] : "";
    data["city"] = parts.size() > 1 ? parts[1] : "";
    data["district"] = parts.size() > 2 ? parts[2] : "";

    data["detail"] = m_tableWidget->item(currentRow, 4)->text();
    data["isDefault"] = (m_tableWidget->item(currentRow, 5)->text() == "Yes");

    showAddressDialog(addressId, data);
}

void AddressWidget::onDeleteClicked()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Tip", "Please select an address to delete first");
        return;
    }

    int addressId = m_tableWidget->item(currentRow, 0)->text().toInt();
    QString name = m_tableWidget->item(currentRow, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        QString("Are you sure you want to delete the address \"%1\"?").arg(name),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        emit deleteAddressRequested(addressId);
    }
}

void AddressWidget::onSetDefaultClicked()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Tip", "Please select an address to set as default first");
        return;
    }

    int addressId = m_tableWidget->item(currentRow, 0)->text().toInt();
    emit setDefaultAddressRequested(addressId);
}

void AddressWidget::onRefreshClicked()
{
    emit refreshRequested();
}

// ========== 后端调用的槽 ==========

void AddressWidget::onAddressLoaded(const QList<QVariantMap>& addresses)
{
    m_tableWidget->setRowCount(addresses.size());
    for (int i = 0; i < addresses.size(); i++) {
        const QVariantMap& addr = addresses[i];

        QString fullLocation = QString("%1 / %2 / %3")
                                   .arg(addr["province"].toString())
                                   .arg(addr["city"].toString())
                                   .arg(addr["district"].toString());

        m_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(addr["id"].toInt())));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(addr["name"].toString()));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(addr["phone"].toString()));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(fullLocation));
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(addr["detail"].toString()));
        m_tableWidget->setItem(i, 5, new QTableWidgetItem(addr["isDefault"].toBool() ? "Yes" : "No"));

        // 操作按钮
        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(5, 2, 5, 2);
        actionLayout->setSpacing(8);

        QPushButton* editBtn = new QPushButton("Edit");
        QPushButton* deleteBtn = new QPushButton("Delete");
        QPushButton* defaultBtn = new QPushButton("Default");

        editBtn->setFixedSize(50, 28);
        deleteBtn->setFixedSize(50, 28);
        defaultBtn->setFixedSize(50, 28);

        editBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 3px; font-size: 11px; }");
        deleteBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 3px; font-size: 11px; }");
        defaultBtn->setStyleSheet("QPushButton { background-color: #E6A23C; color: white; border-radius: 3px; font-size: 11px; }");

        int addrId = addr["id"].toInt();

        connect(editBtn, &QPushButton::clicked, [this, addrId, addr]() {
            showAddressDialog(addrId, addr);
        });
        connect(deleteBtn, &QPushButton::clicked, [this, addrId]() {
            emit deleteAddressRequested(addrId);
        });
        connect(defaultBtn, &QPushButton::clicked, [this, addrId]() {
            emit setDefaultAddressRequested(addrId);
        });

        actionLayout->addWidget(editBtn);
        actionLayout->addWidget(deleteBtn);
        actionLayout->addWidget(defaultBtn);
        actionLayout->addStretch();

        m_tableWidget->setCellWidget(i, 6, actionWidget);
    }
}

void AddressWidget::onOperationSuccess(const QString& message)
{
    QMessageBox::information(this, "Success", message);
    emit refreshRequested();
}

void AddressWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "Failed", error);
}