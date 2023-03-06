#include <QVBoxLayout>
#include <QGroupBox>
#include <QStaticText>
#include <QLabel>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QSerialPortInfo>
#include <QAbstractItemView>

#include <stdexcept>

#include "sweepdialog.h"

static const double c_MaxBaseCurrent = 5000.0;
static const double c_MaxCollectorVoltage = 20.0;
static const int c_MaxPoints = 1000;

SweepPage::Base::Base(QWidget *parent) : QWidget(parent)
{        
    auto layout = new QGridLayout();

    m_baseStartEdit = new QLineEdit();
    m_baseStopEdit  = new QLineEdit();
    m_collectorVoltageEdit  = new QLineEdit();

    m_baseStartEdit->setValidator(new QDoubleValidator(0.0, c_MaxBaseCurrent, 3));
    m_baseStopEdit->setValidator(new QDoubleValidator(0.0, c_MaxBaseCurrent, 3));
    m_collectorVoltageEdit->setValidator(new QDoubleValidator(0.0, c_MaxCollectorVoltage, 3));

    layout->addWidget(new QLabel("Base start current"), 0,0);
    layout->addWidget(m_baseStartEdit, 0,1);
    layout->addWidget(new QLabel("mA"), 0,2);

    layout->addWidget(new QLabel("Base stop current"), 1,0);
    layout->addWidget(m_baseStopEdit, 1,1);
    layout->addWidget(new QLabel("mA"), 1,2);

    layout->addWidget(new QLabel("Collector voltage"), 2,0);
    layout->addWidget(m_collectorVoltageEdit, 2,1);
    layout->addWidget(new QLabel("V"), 2,2);

    setLayout(layout);
}

Messages::SweepBase SweepPage::Base::get() const noexcept
{
    Messages::SweepBase data;
    data.m_collectorVoltage=0;
    data.m_startCurrent=0;
    data.m_stopCurrent=0;

    try
    {
        data.m_collectorVoltage = std::stof(m_collectorVoltageEdit->text().toStdString());
        data.m_startCurrent = std::stof(m_baseStartEdit->text().toStdString());
        data.m_stopCurrent  = std::stof(m_baseStopEdit->text().toStdString());
        return data;
    }
    catch(std::invalid_argument(const std::string &what))
    {
        return data;
    }
}

void SweepPage::Base::populate(const Messages::SweepBase &base)
{
    m_baseStartEdit->setText(QString::number(base.m_startCurrent));
    m_baseStopEdit->setText(QString::number(base.m_stopCurrent));
    m_collectorVoltageEdit->setText(QString::number(base.m_collectorVoltage));
}

SweepPage::Collector::Collector(QWidget *parent) : QWidget(parent)
{        
    auto layout = new QGridLayout();

    m_collectorStartEdit = new QLineEdit();
    m_collectorStopEdit  = new QLineEdit();
    m_baseCurrentEdit  = new QLineEdit();

    m_collectorStartEdit->setValidator(new QDoubleValidator(0.0, c_MaxCollectorVoltage, 3));
    m_collectorStopEdit->setValidator(new QDoubleValidator(0.0, c_MaxCollectorVoltage, 3));
    m_baseCurrentEdit->setValidator(new QDoubleValidator(0.0, c_MaxBaseCurrent, 3));

    layout->addWidget(new QLabel("Collector start voltage"), 0,0);
    layout->addWidget(m_collectorStartEdit, 0,1);
    layout->addWidget(new QLabel("V"), 0,2);

    layout->addWidget(new QLabel("Collector stop voltage"), 1,0);
    layout->addWidget(m_collectorStopEdit, 1,1);
    layout->addWidget(new QLabel("V"), 1,2);

    layout->addWidget(new QLabel("Base current"), 2,0);
    layout->addWidget(m_baseCurrentEdit, 2,1);
    layout->addWidget(new QLabel("mA"), 2,2);

    setLayout(layout);
}

Messages::SweepCollector SweepPage::Collector::get() const noexcept
{
    Messages::SweepCollector data;
    data.m_startVoltage=0;
    data.m_stopVoltage=0;
    data.m_baseCurrent=0;
    
    try
    {
        data.m_startVoltage = std::stof(m_collectorStartEdit->text().toStdString());
        data.m_stopVoltage  = std::stof(m_collectorStopEdit->text().toStdString());
        data.m_baseCurrent  = std::stof(m_baseCurrentEdit->text().toStdString());
        return data;
    }
    catch(std::invalid_argument(const std::string &what))
    {
        return data;
    }
}

void SweepPage::Collector::populate(const Messages::SweepCollector &collector)
{
    m_collectorStartEdit->setText(QString::number(collector.m_startVoltage));
    m_collectorStopEdit->setText(QString::number(collector.m_stopVoltage));
    m_baseCurrentEdit->setText(QString::number(collector.m_baseCurrent));
}

SweepPage::Diode::Diode(QWidget *parent) : QWidget(parent)
{        
    auto layout = new QGridLayout();

    m_currentStartEdit = new QLineEdit();
    m_currentStopEdit  = new QLineEdit();

    m_currentStartEdit->setValidator(new QDoubleValidator(0.0, c_MaxBaseCurrent, 3));
    m_currentStopEdit->setValidator(new QDoubleValidator(0.0, c_MaxBaseCurrent, 3));

    layout->addWidget(new QLabel("Start current"), 0,0);
    layout->addWidget(m_currentStartEdit, 0,1);
    layout->addWidget(new QLabel("mA"), 0,2);

    layout->addWidget(new QLabel("Stop current"), 1,0);
    layout->addWidget(m_currentStopEdit, 1,1);
    layout->addWidget(new QLabel("mA"), 1,2);

    setLayout(layout);
}

Messages::SweepDiode SweepPage::Diode::get() const noexcept
{
    Messages::SweepDiode data;
    data.m_startCurrent=0;
    data.m_stopCurrent=0;

    try
    {
        data.m_startCurrent = std::stof(m_currentStartEdit->text().toStdString());
        data.m_stopCurrent  = std::stof(m_currentStopEdit->text().toStdString());
        return data;
    }
    catch(std::invalid_argument(const std::string &what))
    {
        return data;
    }
}

void SweepPage::Diode::populate(const Messages::SweepDiode &diode)
{
    m_currentStartEdit->setText(QString::number(diode.m_startCurrent));
    m_currentStopEdit->setText(QString::number(diode.m_stopCurrent));
}


SweepDialog::SweepDialog(const Messages::SweepSetup &setup, QWidget *parent) : QDialog(parent), m_setup(setup)
{
    setWindowTitle("Sweep setup");

    auto mainLayout = new QVBoxLayout();

    m_tabs = new QTabWidget();
    mainLayout->addWidget(m_tabs);

    m_diodeSweep = new SweepPage::Diode();
    m_tabs->addTab(m_diodeSweep, "Diode");

    m_collectorSweep = new SweepPage::Collector();
    m_tabs->addTab(m_collectorSweep, "Collector");

    m_baseSweep = new SweepPage::Base();
    m_tabs->addTab(m_baseSweep, "Base");

    m_baseSweep->populate(m_setup.m_base);
    m_collectorSweep->populate(m_setup.m_collector);
    m_diodeSweep->populate(m_setup.m_diode);

    auto hbox = new QHBoxLayout();
    m_numPointsEdit = new QLineEdit();
    m_numPointsEdit->setValidator(new QIntValidator(0, 1000));

    hbox->addWidget(new QLabel("Number of points"));
    hbox->addWidget(m_numPointsEdit);

    mainLayout->addLayout(hbox, 1);

    m_numPointsEdit->setText(QString::number(setup.m_points));

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SweepDialog::onOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

void SweepDialog::onOk()
{
    m_setup.m_base = m_baseSweep->get();
    m_setup.m_collector = m_collectorSweep->get();
    m_setup.m_diode = m_diodeSweep->get();
    m_setup.m_points = std::stol(m_numPointsEdit->text().toStdString());

    QDialog::accept();
}
