#include <QVBoxLayout>
#include <QStaticText>
#include <QLabel>
#include <QDialogButtonBox>
#include <QSerialPortInfo>
#include <QAbstractItemView>

#include "sweepdialog.h"

SweepDialog::SweepDialog(const SweepSetup &setup, QWidget *parent) : QDialog(parent), m_setup(setup)
{
    setWindowTitle("Sweep setup");

    auto mainLayout = new QVBoxLayout();

    auto setupLayout = new QGridLayout();
    setupLayout->addWidget(new QLabel(tr("Base sense resistor")), 0, 0);
    setupLayout->addWidget(new QLabel(tr("kOhm")), 0, 2);
    setupLayout->addWidget(new QLabel(tr("Base limit resistor")), 1, 0);
    setupLayout->addWidget(new QLabel(tr("kOhm")), 1, 2);
    setupLayout->addWidget(new QLabel(tr("Max. base current")), 2, 0);
    setupLayout->addWidget(new QLabel(tr("µA")), 2, 2);
    setupLayout->addWidget(new QLabel(tr("Collector resistor")), 3, 0);
    setupLayout->addWidget(new QLabel(tr("kOhm")), 3, 2);
    setupLayout->addWidget(new QLabel(tr("Base current start")), 4, 0);
    setupLayout->addWidget(new QLabel(tr("µA")), 4, 2);
    setupLayout->addWidget(new QLabel(tr("Base current stop")), 5, 0);
    setupLayout->addWidget(new QLabel(tr("µA")), 5, 2);
    setupLayout->addWidget(new QLabel(tr("Number of sweeps")), 6, 0);

    m_maxBaseLabel = new QLabel();
    setupLayout->addWidget(m_maxBaseLabel, 2, 1);
    
    m_baseResistorEdit = new QLineEdit(QString::asprintf("%.3f", m_setup.m_baseSenseResistor));
    m_baseLimitResistorEdit = new QLineEdit(QString::asprintf("%.3f", m_setup.m_baseLimitResistor));
    m_collectorResistorEdit = new QLineEdit(QString::asprintf("%.3f", m_setup.m_collectorResistor));

    m_baseResistorEdit->setValidator(new QDoubleValidator(0.0, 1000.0, 3));
    m_baseLimitResistorEdit->setValidator(new QDoubleValidator(0.0, 1000.0, 3));
    m_collectorResistorEdit->setValidator(new QDoubleValidator(0.0, 1000.0, 3));

    connect(m_baseResistorEdit, &QLineEdit::textChanged, this, &SweepDialog::updateMaxBaseLabel);
    connect(m_baseLimitResistorEdit, &QLineEdit::textChanged, this, &SweepDialog::updateMaxBaseLabel);

    setupLayout->addWidget(m_baseResistorEdit, 0,1);
    setupLayout->addWidget(m_baseLimitResistorEdit, 1,1);
    setupLayout->addWidget(m_collectorResistorEdit, 3,1);

    m_baseStartEdit = new QLineEdit(QString::asprintf("%d", m_setup.m_baseCurrentStart));
    m_baseStopEdit  = new QLineEdit(QString::asprintf("%d", m_setup.m_baseCurrentStop));
    m_numSweepsEdit = new QLineEdit(QString::asprintf("%d", m_setup.m_numberOfTraces));

    m_baseStartEdit->setValidator(new QIntValidator(0.0, 1000));
    m_baseStopEdit->setValidator(new QIntValidator(0.0, 1000));
    m_numSweepsEdit->setValidator(new QIntValidator(1, 100));

    setupLayout->addWidget(m_baseStartEdit, 4,1);
    setupLayout->addWidget(m_baseStopEdit, 5,1);
    setupLayout->addWidget(m_numSweepsEdit, 6,1);

    updateMaxBaseLabel();

    mainLayout->addLayout(setupLayout);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SweepDialog::onOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    //mainLayout->addWidget(m_portList);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

void SweepDialog::updateMaxBaseLabel()
{    
    float totalResistance = 0.0f;
    totalResistance += m_baseResistorEdit->text().toDouble();
    totalResistance += m_baseLimitResistorEdit->text().toDouble();

    const float voltage = 5.0f;
    const float baseDropVolts = 0.6f;
    auto currentInMicroamps = static_cast<int>(1000.0f*(voltage-baseDropVolts) / totalResistance);

    m_maxBaseLabel->setText(QString::asprintf("%d", currentInMicroamps));
}

void SweepDialog::onOk()
{
    m_setup.m_baseCurrentStart = m_baseStartEdit->text().toInt();
    m_setup.m_baseCurrentStop  = m_baseStopEdit->text().toInt();
    m_setup.m_baseLimitResistor  = m_baseLimitResistorEdit->text().toDouble();
    m_setup.m_baseSenseResistor  = m_baseResistorEdit->text().toDouble();
    m_setup.m_collectorResistor  = m_collectorResistorEdit->text().toDouble();
    m_setup.m_numberOfTraces  = m_numSweepsEdit->text().toInt();

    QDialog::accept();
}
