#pragma once
#include <string>
#include <memory>
#include <queue>
#include "customevent.h"
#include <QtSerialPort/QSerialPort>
#include <QTimer>

class SerialCtrl : public QObject
{
    Q_OBJECT

public:
    virtual ~SerialCtrl();
    
    static SerialCtrl* open(const std::string &devname, QObject *eventReceiver);

    void sweepCollector(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step);
    void sweepBase(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step);
    void sweepDiode(uint16_t dutyStart, uint16_t dutyEnd, uint16_t step);
        
    void setBasePWM(uint16_t dutyCycle, bool noMeasurement = false);
    void setCollectorPWM(uint16_t dutyCycle, bool noMeasurement = false);
    void setDiodePWM(uint16_t dutyCycle, bool noMeasurement = false);

    bool isOpen() const;
    void close();

    void run();

protected slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);
    void handleTimeout();

    void onTimer();

protected:
    SerialCtrl(QSerialPort *port, QObject *eventReceiver);
    
    void startSweep();
    void endSweep();

    void transmitCommand();

    constexpr bool isEOL(char c) const noexcept
    {
        return ((c==10) || (c==13));
    }

    constexpr bool isAcceptableChar(char c) const noexcept
    {
        return ((c >= '0') && (c <= '9')) || (c=='\t') || (c==' ');
    }

    std::unique_ptr<QSerialPort> m_port;
    QObject *m_eventReceiver;

    enum class CommandType
    {
        INVALID = 0,
        SETBASEPWM,
        SETCOLLECTORPWM,
        SETDIODEPWM,
        STARTSWEEP,
        ENDSWEEP
    };

    struct Command
    {
        CommandType m_type;
        int32_t     m_pwm;
        int32_t     m_response[2];
        bool        m_reportResponse;
    };

    std::queue<Command> m_commands;
    bool m_pendingResponse;

    QTimer *m_timer;
};

