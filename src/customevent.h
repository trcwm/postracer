#pragma once

#include <string>
#include <QEvent>

class DataEvent : public QEvent
{
public:

    enum class DataType
    {
        Unknown,
        Base,
        Collector,
        Diode,
        EndSweep
    };

    DataEvent(const std::string &data, DataType type)
        : QEvent(QEvent::User), m_data(data), m_type(type)
    {

    }

    const std::string data() const
    {
        return m_data;
    }

    DataType dataType() const
    {
        return m_type;
    }

private:
    DataType    m_type;
    std::string m_data;
};
