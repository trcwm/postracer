#include <memory>
#include <array>
#include <sstream>
#include <iostream>
#include <thread>
#include <QDebug>
#include <QApplication>
#include <QStringList>
#include "profiling.h"
#include "serialctrl.h"

SerialCtrl::SerialCtrl(QSerialPort *port, QueueType &queue)
    : m_port(port), m_queue(queue)
{
    //connect(m_port.get(), &QSerialPort::readyRead, this, &SerialCtrl::handleReadyRead);
    //connect(m_port.get(), &QSerialPort::errorOccurred, this, &SerialCtrl::handleError);

    // timer used to fix a bug in QSerialPort that causes readyRead to be broken
    // 
#if 0    
    m_timer = new QTimer(this);    
    connect(m_timer, &QTimer::timeout, this, &SerialCtrl::onTimer);
    m_timer->start(50);
#endif

};

SerialCtrl* SerialCtrl::open(const std::string &devname, QueueType &queue)
{
    std::unique_ptr<QSerialPort> port(new QSerialPort());

    port->setPortName(QString::fromStdString(devname));
    port->setBaudRate(QSerialPort::Baud9600);

    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);

    if (!port->open(QIODevice::ReadWrite))
    {
        return nullptr;
    }
    else
    {
        port->flush();
        port->readAll();        
        return new SerialCtrl(port.release(), queue);
    }
}

SerialCtrl::~SerialCtrl()
{
    if (m_port)
    {
        if (m_port->isOpen())
        {
            m_port->close();
        }
    }
}

bool SerialCtrl::isOpen() const
{
    if (!m_port)
    {
        return false;
    }

    return m_port->isOpen();
}

void SerialCtrl::close()
{
    if (m_port)
    {
        m_port->close();
    }
}

void SerialCtrl::sendString(const QString &txt)
{
    if (!m_port) return;

    m_port->write(txt.toUtf8());
    // wait for reply
    if (m_port->waitForBytesWritten(1000))
    {        
    }
    else
    {
        qDebug() << "Serial write time-out";
        return;
    }        
}

std::optional<QString> SerialCtrl::readLine()
{
    QByteArray responseData;
    if (!m_port) return std::nullopt;

    while (m_port->bytesAvailable() || m_port->waitForReadyRead(1000))
    {
        auto byte = m_port->read(1);
        responseData += byte;
        
        if (byte.at(0) == '\n')
        {
            return responseData;
        }
    }
    return std::nullopt;
}

static const float gc_currentShuntR = 1.5f;  // estimated

void SerialCtrl::doSweep(const Messages::SweepSetup &sweep)
{
    std::size_t count = 0;
    Profiling::Timer profiling("doSweep");

    switch(sweep.m_sweepType)
    {
    case Messages::SweepType::Diode:
        if (m_sweepThread != nullptr)
        {
            m_sweepThread->join();
            delete m_sweepThread;
        }
        
        m_sweepThread = new std::thread([this,sweep](){sweepDiode(sweep);});
        break;
    case Messages::SweepType::BJT_Base:
        if (m_sweepThread != nullptr)
        {
            m_sweepThread->join();
            delete m_sweepThread;
        }
        
        m_sweepThread = new std::thread([this,sweep](){sweepBJT_Base(sweep);});
        break;
    case Messages::SweepType::BJT_Collector:
        if (m_sweepThread != nullptr)
        {
            m_sweepThread->join();
            delete m_sweepThread;
        }
        
        m_sweepThread = new std::thread([this,sweep](){sweepBJT_VCE(sweep);});
        break;
    default:
        qDebug() << "Invalid sweep type";
        break;
    }
}

void SerialCtrl::setBaseCurrent(float amperes)
{
    auto microAmps = static_cast<int>(amperes * 1e6f);

    if (microAmps > 5000) microAmps = 5000; 
    if (microAmps < 0) microAmps = 0; 

    QString cmd = "B";
    cmd.append(QString::number(microAmps));
    cmd.append("\n\r");

    sendString(cmd);

    auto response = readLine();
    if (!response)
    {
        qDebug() << "Serial write time-out";
        return;
    }

    if (response->at(0) == '-')
    {
        qDebug() << "Error setting base current";
    }

    //qDebug() << "  response: " << response.value();
    //qDebug() << "Base current set to " << amperes * 1e6 << " uA";
}

void SerialCtrl::setCollectorVoltage(float volts)
{
    auto milliVolts = static_cast<int>(volts * 1e3f);

    if (milliVolts > 20000) milliVolts = 20000; 
    if (milliVolts < 0) milliVolts = 0; 

    QString cmd = "C";
    cmd.append(QString::number(milliVolts));
    cmd.append("\n\r");
    sendString(cmd);

    auto response = readLine();
    if (!response)
    {
        qDebug() << "Serial write time-out";
        return;
    }

    //qDebug() << "  response: " << response.value();

    if (response->at(0) == '-')
    {
        qDebug() << "Error setting collector voltage";
    }
}

SerialCtrl::MeasureResult SerialCtrl::measure()
{
    QString cmd = "M\n\r";
    m_port->write(cmd.toUtf8());

    // wait for reply
    if (m_port->waitForBytesWritten(1000))
    {        
    }
    else
    {
        qDebug() << "Serial write time-out";
        return MeasureResult{};
    }

    QStringList responses;
    for(int lines=0; lines<3; lines++)
    {
        auto lineOpt = readLine();
        if (lineOpt)
        {
            //qDebug() << "  line: " << lineOpt.value();
            responses.push_back(lineOpt.value());
        }
        else
        {
            qDebug() << "Time-out reading serial port";
        }
    }

    if (responses.size() != 3)
    {
        qDebug() << "Error while taking measurement.";
        return MeasureResult{};
    }

    if (responses.at(0).at(0) == '-')
    {
        qDebug() << "Error while taking measurement.";
        return MeasureResult{};
    }

    MeasureResult result;
    auto ok      = responses.at(0);
    auto base    = responses.at(1);
    auto emitter = responses.at(2);

    result.m_Base     = base.toFloat() / 10000.0f;
    result.m_Emitter  = emitter.toFloat() / 10000.0f;
    result.m_valid    = true;

    return result;
}


void SerialCtrl::sweepBJT_VCE(const Messages::SweepSetup &sweep)
{
    std::size_t count = 0;

    qDebug() << "Starting BJT collector sweep..";

    float baseCurrent = sweep.m_collector.m_baseCurrents.front();
    setBaseCurrent(baseCurrent);
    
    for(int pt=0; pt<sweep.m_points; pt++)
    {
        count++;

        float frac = static_cast<float>(pt)/(sweep.m_points-1.0f);
        auto voltage = sweep.m_collector.m_startVoltage +
            frac*(sweep.m_collector.m_stopVoltage - sweep.m_collector.m_startVoltage);
        
        setCollectorVoltage(voltage);

        auto data = measure();
        if (data.m_valid)
        {
            Messages::DataPoint p;
            p.m_baseCurrent = baseCurrent;
            p.m_baseVoltage = data.m_Base - data.m_Emitter;
            p.m_collectorVoltage = voltage;
            p.m_emitterCurrent = data.m_Emitter / gc_currentShuntR;
            p.m_emitterVoltage = data.m_Emitter;

            if (count == sweep.m_points)
            {
                std::stringstream ss;
                ss << "Ib = " << baseCurrent * 1000.0f << " mA";
                p.m_label = ss.str();
            }
            m_queue.push(std::move(p));
        }
        else
        {
            qDebug("Collector sweep aborted due to error");
            setBaseCurrent(0);
            setCollectorVoltage(0);                        
            return;
        }
    }
    setBaseCurrent(0);
    setCollectorVoltage(0);
}

void SerialCtrl::sweepBJT_Base(const Messages::SweepSetup &sweep)
{
    std::size_t count = 0;

    qDebug() << "Starting BJT base sweep..";

    float collectorVoltage = sweep.m_base.m_collectorVoltage;
    setCollectorVoltage(collectorVoltage);
    
    for(int pt=0; pt<sweep.m_points; pt++)
    {
        count++;

        float frac = static_cast<float>(pt)/(sweep.m_points-1.0f);
        auto baseCurrent = sweep.m_base.m_startCurrent +
            frac*(sweep.m_base.m_stopCurrent - sweep.m_base.m_startCurrent);
        
        setBaseCurrent(baseCurrent);

        auto data = measure();
        if (data.m_valid)
        {
            Messages::DataPoint p;
            p.m_baseCurrent = baseCurrent;
            p.m_baseVoltage = data.m_Base - data.m_Emitter;
            p.m_collectorVoltage = collectorVoltage;
            p.m_emitterCurrent = data.m_Emitter / gc_currentShuntR;
            p.m_emitterVoltage = data.m_Emitter;

            if (count == sweep.m_points)
            {
                std::stringstream ss;
                ss << "Vce = " << collectorVoltage << " V";
                p.m_label = ss.str();
            }
            m_queue.push(std::move(p));
        }
        else
        {
            qDebug("Base sweep aborted due to error");
            setBaseCurrent(0);
            setCollectorVoltage(0);                        
            return;
        }
    }
    setBaseCurrent(0);
    setCollectorVoltage(0);
}

void SerialCtrl::sweepDiode(const Messages::SweepSetup &sweep)
{
    qDebug() << "Starting diode sweep..";
    for(int pt=0; pt<sweep.m_points; pt++)
    {
        float frac = static_cast<float>(pt)/(sweep.m_points-1.0f);
        auto current = sweep.m_diode.m_startCurrent +
            frac*(sweep.m_diode.m_stopCurrent - sweep.m_diode.m_startCurrent);

        setBaseCurrent(current);
        auto data = measure();
        if (data.m_valid)
        {
            Messages::DataPoint p;
            p.m_baseCurrent = current;
            p.m_baseVoltage = data.m_Base - data.m_Emitter;
            p.m_collectorVoltage = 0; // not applicable
            p.m_emitterCurrent = data.m_Emitter / gc_currentShuntR;
            p.m_emitterVoltage = data.m_Emitter;
            m_queue.push(std::move(p));      
        }
        else
        {
            qDebug("Diode sweep aborted due to error");
            setBaseCurrent(0);
            setCollectorVoltage(0);
            return;
        }
    }
    setBaseCurrent(0);
    setCollectorVoltage(0);
}
