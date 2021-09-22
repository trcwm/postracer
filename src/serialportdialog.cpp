#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QSerialPortInfo>
#include <QAbstractItemView>

#include "serialportdialog.h"

SerialPortDialog::SerialPortDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Select serial port");

    auto mainLayout = new QVBoxLayout();

    m_portList = new QListWidget();
    m_portList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);

    // populate port list
    auto list = QSerialPortInfo::availablePorts();

    for(auto const& p : list)
    {
        auto item = new QListWidgetItem();
        item->setData(Qt::ItemDataRole::DisplayRole, p.portName());
        item->setData(Qt::ItemDataRole::UserRole, p.systemLocation());
        m_portList->addItem(item);
    }

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(m_portList);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

QString SerialPortDialog::getSerialPortLocation() const
{
    auto curItem = m_portList->currentItem();
    if (curItem == nullptr)
    {
        return "";
    }

    return curItem->data(Qt::ItemDataRole::UserRole).toString();
}
