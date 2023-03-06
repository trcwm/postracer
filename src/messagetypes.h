#pragma once
#include <string>

namespace Messages
{
    struct DataPoint
    {
        float m_baseCurrent;            ///< Amperes
        float m_baseVoltage;            ///< Volts
        float m_collectorVoltage;       ///< Volts
        float m_emitterCurrent;         ///< Amperes
        std::string m_version;          ///< verion string, if requested
    };

    enum class SweepType
    {
        Undefined,
        Diode,
        BJT_Collector,
        BJT_Base
    };

    /** sweep the diode current */
    struct SweepDiode
    {
        float m_startCurrent;
        float m_stopCurrent;
    };

    /** sweep collector voltage while having a constant base current */
    struct SweepCollector
    {
        float m_startVoltage;
        float m_stopVoltage;
        float m_baseCurrent; 
    };

    /** sweep the base current while having a constant collector voltage */
    struct SweepBase
    {
        float m_startCurrent;
        float m_stopCurrent;
        float m_collectorVoltage;
    };

    struct SweepSetup
    {
        SweepType m_sweepType = SweepType::Undefined;

        // Note: don't make this a union because
        // the UI dialog needs to have separate
        // data for each.
        SweepDiode      m_diode;
        SweepCollector  m_collector;
        SweepBase       m_base;
        uint16_t        m_points{0};
    };

};
