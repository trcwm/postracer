#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QSerialPortInfo>
#include <QAbstractItemView>

#include "sweepdialog.h"

SweepDialog::SweepDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Sweep setup");

    auto mainLayout = new QVBoxLayout();

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    //mainLayout->addWidget(m_portList);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}
