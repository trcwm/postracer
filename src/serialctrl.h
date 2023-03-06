#pragma once
#include <string>
#include <memory>
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

protected slots:
    //void handleReadyRead();
    //void handleError(QSerialPort::SerialPortError error);
    //void handleTimeout();

protected:
    SerialCtrl(QSerialPort *port, QueueType &queue);
    
    constexpr bool isEOL(char c) const noexcept
    {
        return ((c==10) || (c==13));
    }

    constexpr bool isAcceptableChar(char c) const noexcept
    {
        return ((c >= '0') && (c <= '9')) || (c=='\t') || (c==' ');
    }

    QueueType &m_queue;

    std::unique_ptr<QSerialPort> m_port;
};

