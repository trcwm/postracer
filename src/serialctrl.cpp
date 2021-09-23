#include <memory>
#include <array>
#include <sstream>
#include <iostream>
#include <thread>
#include <QApplication>
#include "serialctrl.h"

SerialCtrl::SerialCtrl(QSerialPort *port, QObject *eventReceiver)
    : m_port(port), m_eventReceiver(eventReceiver)
{
    m_pendingResponse = false;
    connect(m_port.get(), &QSerialPort::readyRead, this, &SerialCtrl::handleReadyRead);
    connect(m_port.get(), &QSerialPort::errorOccurred, this, &SerialCtrl::handleError);

    // timer used to fix a bug in QSerialPort that causes readyRead to be broken
    // 
#if 0    
    m_timer = new QTimer(this);    
    connect(m_timer, &QTimer::timeout, this, &SerialCtrl::onTimer);
    m_timer->start(50);
#endif

};

SerialCtrl* SerialCtrl::open(const std::string &devname, QObject *eventReceiver)
{
    std::unique_ptr<QSerialPort> port(new QSerialPort());

    port->setPortName(QString::fromStdString(devname));
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);

    if (!port->open(QIODevice::ReadWrite))
    {
        return nullptr;
    }
    else
    {
        port->readAll();
        return new SerialCtrl(port.release(), eventReceiver);
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

void SerialCtrl::run()
{
    if ((!m_pendingResponse) && m_port)
    {
        transmitCommand();
    }
}

void SerialCtrl::transmitCommand()
{
    if (m_commands.empty())
    {
        #ifdef DEBUGSERIAL
        std::cout << "TransmitCommand: queue empty\n";
        #endif
        return;
    }

    auto cmd = m_commands.front();

#ifdef DEBUGSERIAL    
    switch(cmd.m_type)
    {
    case CommandType::SETBASEPWM:
        std::cout << "TransmitCommand: SETBASEPWM\n";
        break;
    case CommandType::SETCOLLECTORPWM:
        std::cout << "TransmitCommand: SETCOLLECTORPWM\n";
        break;
    case CommandType::SETDIODEPWM:
        std::cout << "TransmitCommand: SETDIODEPWM\n";
        break;
    case CommandType::STARTSWEEP:
        std::cout << "TransmitCommand: STARTSWEEP\n";
        break;
    case CommandType::ENDSWEEP:
        std::cout << "TransmitCommand: ENDSWEEP\n";
        break;        
    }
#endif    

    std::stringstream ss;
    std::string txstr;
    switch(cmd.m_type)
    {
    case CommandType::SETBASEPWM:
        ss << cmd.m_pwm << "B \n";
        txstr = ss.str();
        m_port->write(txstr.c_str(), txstr.size());
        m_port->flush();
        m_pendingResponse = true;
        break;
    case CommandType::SETCOLLECTORPWM:
        ss << cmd.m_pwm << "C \n";
        txstr = ss.str();
        m_port->write(txstr.c_str(), txstr.size());
        m_port->flush();
        m_pendingResponse = true;
        break; 
    case CommandType::SETDIODEPWM:
        ss << cmd.m_pwm << "C \n";
        m_port->write(txstr.c_str(), txstr.size());
        m_port->flush();
        m_pendingResponse = true;
        break;         
    case CommandType::STARTSWEEP:
        QApplication::postEvent(m_eventReceiver, new DataEvent("", DataEvent::DataType::StartSweep));
        m_commands.pop();
        transmitCommand();
        break;
    case CommandType::ENDSWEEP:
        QApplication::postEvent(m_eventReceiver, new DataEvent("", DataEvent::DataType::EndSweep));
        m_commands.pop();
        transmitCommand();
        break;        
    default:
        // invalid command!
        break;
    }
}

void SerialCtrl::setBaseCurrent(uint16_t dutyCycle, bool noMeasurement)
{
    Command cmd;
    cmd.m_pwm = dutyCycle;
    cmd.m_reportResponse = !noMeasurement;
    cmd.m_type = CommandType::SETBASEPWM;
    cmd.m_response[0] = 0;
    cmd.m_response[1] = 0;

    m_commands.push(cmd);
}

void SerialCtrl::setCollectorVoltage(uint16_t dutyCycle, bool noMeasurement)
{
    Command cmd;
    cmd.m_pwm = dutyCycle;
    cmd.m_reportResponse = !noMeasurement;
    cmd.m_type = CommandType::SETCOLLECTORPWM;
    cmd.m_response[0] = 0;
    cmd.m_response[1] = 0;

    m_commands.push(cmd);  
}

void SerialCtrl::setDiodeVoltage(uint16_t dutyCycle, bool noMeasurement)
{
    Command cmd;
    cmd.m_pwm = dutyCycle;
    cmd.m_reportResponse = !noMeasurement;
    cmd.m_type = CommandType::SETDIODEPWM;
    cmd.m_response[0] = 0;
    cmd.m_response[1] = 0;

    m_commands.push(cmd);
}


void SerialCtrl::startSweep()
{
    Command cmd;
    cmd.m_reportResponse = true;
    cmd.m_type = CommandType::STARTSWEEP;
    m_commands.push(cmd);
}

void SerialCtrl::endSweep()
{
    Command cmd;
    cmd.m_reportResponse = true;
    cmd.m_type = CommandType::ENDSWEEP;
    m_commands.push(cmd);
}


void SerialCtrl::sweepCollector(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step)
{
    startSweep();
    for(uint16_t duty = dutyStart; duty <= dutyEnd; duty += step)
    {
        setCollectorVoltage(duty);        
    }
    endSweep();
}

void SerialCtrl::sweepDiode(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step)
{
    startSweep();
    for(uint16_t duty = dutyStart; duty <= dutyEnd; duty += step)
    {
        setDiodeVoltage(duty);
    }
    endSweep();
}


void SerialCtrl::handleReadyRead()
{
    auto buf = m_port->readAll();

    //std::cout << "serial read: '" << buf.toStdString() << "'\n";

    std::string response;
    for(char c : buf)
    {
        if (isAcceptableChar(c))
        {
            response += c;
        }
        
        if (isEOL(c))
        {
            break;
        }
    }

    auto cmd = m_commands.front();
    m_commands.pop();

    if (cmd.m_reportResponse)
    {
        switch(cmd.m_type)       
        {
        case CommandType::SETCOLLECTORPWM:
            QApplication::postEvent(m_eventReceiver, new DataEvent(response, DataEvent::DataType::Collector));
            break;
        case CommandType::SETBASEPWM:
            QApplication::postEvent(m_eventReceiver, new DataEvent(response, DataEvent::DataType::Base));
            break;
        case CommandType::SETDIODEPWM:
            QApplication::postEvent(m_eventReceiver, new DataEvent(response, DataEvent::DataType::Diode));
            break;
        }
    }

    //std::cout << "Response RX: " << response << " queue=" << m_commands.size() << "\n";
    m_pendingResponse = false;

    run();  // trigger next command TX
}

void SerialCtrl::handleError(QSerialPort::SerialPortError error)
{
#ifdef DEBUGSERIAL    
    switch(error)
    {
    case QSerialPort::OpenError:
        std::cout << "Serial open error\n";
        break;
    case QSerialPort::ReadError:
        std::cout << "Serial read error\n";
        break;
    case QSerialPort::WriteError:
        std::cout << "Serial write error\n";
        break;       
    case QSerialPort::TimeoutError:
        //std::cout << "Serial time-out error\n";
        break;        
    default:
        std::cout << "Serial error " << error << "\n";
        break;                
    }
#endif
}

void SerialCtrl::handleTimeout()
{

}

void SerialCtrl::onTimer()
{
    if (m_port)
    {
        m_port->waitForReadyRead(1);
    }
}
