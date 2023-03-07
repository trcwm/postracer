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
        float m_emitterVoltage;         ///< Volts
        std::string m_version;          ///< verion string, if requested
        std::string m_label;            ///< label text to add at end of sweep
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
        float m_startCurrent;   ///< Amperes (max 5 mA)
        float m_stopCurrent;    ///< Amperes (max 5 mA)
    };

    /** sweep collector voltage while having a constant base current */
    struct SweepCollector
    {
        float m_startVoltage;   ///< Volts (max 20 V)
        float m_stopVoltage;    ///< Volts (max 20 V)
        std::vector<float> m_baseCurrents;    ///< Amperes (max 5 mA)
    };

    /** sweep the base current while having a constant collector voltage */
    struct SweepBase
    {
        float m_startCurrent;       ///< Amperes (max 5 mA)
        float m_stopCurrent;        ///< Amperes (max 5 mA)
        float m_collectorVoltage;   ///< Volts (max 20 V)
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
