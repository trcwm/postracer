#pragma once
#include <string>
#include <memory>
#include <optional>

#include <QtSerialPort/QSerialPort>
#include <QTimer>

#include "messagequeue.h"
#include "messagetypes.h"

class SerialCtrl : public QObject
{
    Q_OBJECT

public:
    using QueueType = MessageQueue<Messages::DataPoint>;

    virtual ~SerialCtrl();
    
    static SerialCtrl* open(const std::string &devname, QueueType &queue);

    void doSweep(const Messages::SweepSetup &sweep);
        
    bool isOpen() const;
    void close();

protected:
    SerialCtrl(QSerialPort *port, QueueType &queue);
    
    void sweepDiode(const Messages::SweepSetup &sweep);
    void sweepBJT_VCE(const Messages::SweepSetup &sweep);
    void sweepBJT_Base(const Messages::SweepSetup &sweep);

    constexpr bool isEOL(char c) const noexcept
    {
        return ((c==10) || (c==13));
    }

    constexpr bool isAcceptableChar(char c) const noexcept
    {
        return ((c >= '0') && (c <= '9')) || (c=='\t') || (c==' ');
    }

    void sendString(const QString &txt);
    std::optional<QString> readLine();

    struct MeasureResult
    {
        bool  m_valid = false;
        float m_VREF_5V0;
        float m_VREF_2V5;
        float m_Emitter;    ///< emitter current shunt voltage
        float m_Base;       ///< base voltage
    };

    void setBaseCurrent(float amperes);
    void setCollectorVoltage(float volts);
    MeasureResult measure();

    QueueType &m_queue;
    std::unique_ptr<QSerialPort> m_port;
    std::thread *m_sweepThread = nullptr;
};

