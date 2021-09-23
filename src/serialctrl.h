#pragma once
#include <string>
#include <memory>
#include "customevent.h"
#include <QtSerialPort/QSerialPort>

class SerialCtrl
{
public:
    virtual ~SerialCtrl();
    
    static SerialCtrl* open(const std::string &devname, QObject *eventReceiver);

    void setBaseCurrent(uint16_t dutyCycle, bool noMeasurement = false);
    void setCollectorVoltage(uint16_t dutyCycle, bool noMeasurement = false);
    void setDiodeVoltage(uint16_t dutyCycle, bool noMeasurement = false);

    void sweepCollector(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step);
    void sweepDiode(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step);

    bool isOpen() const;
    void close();

protected:
    SerialCtrl(QSerialPort *port, QObject *eventReceiver) 
        : m_port(port), m_eventReceiver(eventReceiver) {};

    constexpr bool isEOL(char c) const noexcept
    {
        return ((c==10) || (c==13));
    }

    constexpr bool isAcceptableChar(char c) const noexcept
    {
        return ((c >= '0') && (c <= '9')) || (c=='\t') || (c==' ');
    }

    std::string readLine();
    std::unique_ptr<QSerialPort> m_port;
    QObject *m_eventReceiver;
};

