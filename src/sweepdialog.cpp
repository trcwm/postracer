#include <QVBoxLayout>
#include <QGroupBox>
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

    auto deviceBox = new QGroupBox("Device configuration");

    auto configLayout = new QGridLayout();
    configLayout->addWidget(new QLabel(tr("Base sense resistor")), 0, 0);
    configLayout->addWidget(new QLabel(tr("kOhm")), 0, 2);
    configLayout->addWidget(new QLabel(tr("Base limit resistor")), 1, 0);
    configLayout->addWidget(new QLabel(tr("kOhm")), 1, 2);
    configLayout->addWidget(new QLabel(tr("Max. base current")), 2, 0);
    configLayout->addWidget(new QLabel(tr("µA")), 2, 2);
    configLayout->addWidget(new QLabel(tr("Collector resistor")), 3, 0);
    configLayout->addWidget(new QLabel(tr("kOhm")), 3, 2);

    m_maxBaseLabel = new QLabel();
    configLayout->addWidget(m_maxBaseLabel, 2, 1);
    
    deviceBox->setLayout(configLayout);
    
    
    
    auto sweepBox = new QGroupBox("Sweep setup");

    auto sweepLayout = new QGridLayout();
    sweepLayout->addWidget(new QLabel(tr("Base current start")), 0, 0);
    sweepLayout->addWidget(new QLabel(tr("µA")), 0, 2);
    sweepLayout->addWidget(new QLabel(tr("Base current stop")), 1, 0);
    sweepLayout->addWidget(new QLabel(tr("µA")), 1, 2);
    sweepLayout->addWidget(new QLabel(tr("Number of sweeps")), 2, 0);

    sweepBox->setLayout(sweepLayout);


    m_baseResistorEdit = new QLineEdit(QString::asprintf("%.3f", m_setup.m_baseSenseResistor));
    m_baseLimitResistorEdit = new QLineEdit(QString::asprintf("%.3f", m_setup.m_baseLimitResistor));
    m_collectorResistorEdit = new QLineEdit(QString::asprintf("%.3f", m_setup.m_collectorResistor));

    m_baseResistorEdit->setValidator(new QDoubleValidator(0.0, 1000.0, 3));
    m_baseLimitResistorEdit->setValidator(new QDoubleValidator(0.0, 1000.0, 3));
    m_collectorResistorEdit->setValidator(new QDoubleValidator(0.0, 1000.0, 3));

    connect(m_baseResistorEdit, &QLineEdit::textChanged, this, &SweepDialog::updateMaxBaseLabel);
    connect(m_baseLimitResistorEdit, &QLineEdit::textChanged, this, &SweepDialog::updateMaxBaseLabel);

    configLayout->addWidget(m_baseResistorEdit, 0,1);
    configLayout->addWidget(m_baseLimitResistorEdit, 1,1);
    configLayout->addWidget(m_collectorResistorEdit, 3,1);

    m_baseStartEdit = new QLineEdit(QString::asprintf("%d", m_setup.m_baseCurrentStart));
    m_baseStopEdit  = new QLineEdit(QString::asprintf("%d", m_setup.m_baseCurrentStop));
    m_numSweepsEdit = new QLineEdit(QString::asprintf("%d", m_setup.m_numberOfTraces));

    m_baseStartEdit->setValidator(new QIntValidator(0.0, 1000));
    m_baseStopEdit->setValidator(new QIntValidator(0.0, 1000));
    m_numSweepsEdit->setValidator(new QIntValidator(1, 100));

    sweepLayout->addWidget(m_baseStartEdit, 0,1);
    sweepLayout->addWidget(m_baseStopEdit, 1,1);
    sweepLayout->addWidget(m_numSweepsEdit, 2,1);

    updateMaxBaseLabel();

    mainLayout->addWidget(deviceBox);
    mainLayout->addWidget(sweepBox);

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
