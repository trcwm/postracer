#include <memory>
#include <array>
#include <sstream>
#include <iostream>
#include <QApplication>
#include "serialctrl.h"

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
        // discard all incoming data
        while(port->bytesAvailable() > 0)
        {
            port->read(port->bytesAvailable());
        }
                
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

void SerialCtrl::setBaseCurrent(uint16_t dutyCycle, bool noMeasurement)
{
    if (m_port->isOpen())
    {
        std::stringstream ss;
        ss << dutyCycle << "B \n";
        auto data = ss.str();
        m_port->write(data.c_str(), data.size());
        
        auto response = readLine();
        
        std::cout << "B response: " << response << "\n";

        if (!noMeasurement)
        {
            QApplication::postEvent(m_eventReceiver, new DataEvent(response, DataEvent::DataType::Base));
        }
    }
}

void SerialCtrl::setCollectorVoltage(uint16_t dutyCycle, bool noMeasurement)
{
    if (m_port->isOpen())
    {
        std::stringstream ss;
        ss << dutyCycle << "C \n";
        auto data = ss.str();
        m_port->write(data.c_str(), data.size());

        if (!noMeasurement)
        {
            auto response = readLine();
            QApplication::postEvent(m_eventReceiver, new DataEvent(response, DataEvent::DataType::Collector));
        }
    }
}

void SerialCtrl::setDiodeVoltage(uint16_t dutyCycle, bool noMeasurement)
{
    if (m_port->isOpen())
    {
        std::stringstream ss;
        ss << dutyCycle << "C \n";
        auto data = ss.str();
        m_port->write(data.c_str(), data.size());

        if (!noMeasurement)
        {
            auto response = readLine();
            QApplication::postEvent(m_eventReceiver, new DataEvent(response, DataEvent::DataType::Diode));
        }
    }
}

std::string SerialCtrl::readLine()
{
    std::string response;

    while(true)
    {
        while (!m_port->bytesAvailable())
        {
            //fixme: handle time-out
            if (!m_port->waitForReadyRead(2000))
            {
                std::cerr << "Serial port time-out!\n";
            }
        }

        char c;
        while (m_port->bytesAvailable())
        {
            m_port->getChar(&c);
            if (isAcceptableChar(c))
            {
                response += c;
            }
            else if (isEOL(c))
            {
                // discard the rest..
                while (m_port->bytesAvailable())
                {
                    m_port->getChar(&c);
                }
                return response;
            }
        }
    }
}

void SerialCtrl::sweepCollector(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step)
{
    QApplication::postEvent(m_eventReceiver, new DataEvent("", DataEvent::DataType::StartSweep));
    for(uint16_t duty = dutyStart; duty <= dutyEnd; duty += step)
    {
        setCollectorVoltage(duty);        
    }
    QApplication::postEvent(m_eventReceiver, new DataEvent("", DataEvent::DataType::EndSweep)); // end sweep!
}

void SerialCtrl::sweepDiode(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step)
{
    QApplication::postEvent(m_eventReceiver, new DataEvent("", DataEvent::DataType::StartSweep));
    for(uint16_t duty = dutyStart; duty <= dutyEnd; duty += step)
    {
        setDiodeVoltage(duty);
    }
    QApplication::postEvent(m_eventReceiver, new DataEvent("", DataEvent::DataType::EndSweep)); // end sweep!
}
