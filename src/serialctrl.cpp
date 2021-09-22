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

void SerialCtrl::setBaseCurrent(uint16_t dutyCycle, bool noMeasurement)
{
    if (m_port->isOpen())
    {
        std::stringstream ss;
        ss << dutyCycle << "B \n";
        auto data = ss.str();
        m_port->write(data.c_str(), data.size());
        
        auto response = readLine();
        
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

        auto response = readLine();

        if (!noMeasurement)
        {
            QApplication::postEvent(m_eventReceiver, new DataEvent(response, DataEvent::DataType::Collector));
        }
    }
}

std::string SerialCtrl::readLine()
{
    std::string response;
    
    while(m_port->waitForReadyRead(2000))
    {
        char c;
        while(m_port->getChar(&c))
        {
            response += c;
            if ((c == 10) || (c == 13))
            {
                while(m_port->bytesAvailable() > 0)
                {
                    m_port->getChar(&c);    // discard all the rest..
                }
                return response;
            }
        }
    }

    return response;
}

void SerialCtrl::sweepCollector(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step)
{
    for(uint16_t duty = dutyStart; duty <= dutyEnd; duty += step)
    {
        setCollectorVoltage(duty);        
    }
    QApplication::postEvent(m_eventReceiver, new DataEvent("", DataEvent::DataType::EndSweep)); // end sweep!
}
