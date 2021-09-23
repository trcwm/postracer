#pragma once

#include <string>
#include <QEvent>

struct SweepSetup
{
    float m_baseSenseResistor;  // in kilo ohms
    float m_baseLimitResistor;  // in kilo ohms
    float m_collectorResistor;  // in kilo ohms

    int32_t m_baseCurrentStart; // in microamps
    int32_t m_baseCurrentStop;  // in microamps
    uint32_t m_numberOfTraces;
};

class DataEvent : public QEvent
{
public:

    enum class DataType
    {
        Unknown,
        Base,
        Collector,
        Diode,
        StartSweep,
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
