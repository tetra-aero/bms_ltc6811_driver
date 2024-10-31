#pragma once
#include "Arduino.h"
namespace soc
{
    namespace param
    {
        constexpr double FULL_VOLTAGE = 44.4;
        constexpr double FULL_CAPACITY = 6000.0;                    // mAh
        constexpr double FULL_POWER = FULL_VOLTAGE * FULL_CAPACITY; // mWh
    };
    namespace data
    {
        double soc = 100.0;
        double remain = soc * param::FULL_POWER * 60.0 * 60.0; // mW

        void dbg()
        {
            Serial.println(("# SoC: " + std::to_string(static_cast<float>(soc)) + "[percent]").c_str());
        }
    };
    namespace driver
    {
        void full_recharge()
        {
            data::soc = 100.0;
        }

        double update_soc(float current_mA, float voltage, float delta_ms = 10.0)
        {
            data::remain -= voltage * current_mA * delta_ms / 1000.0;
            data::soc = data::remain / (param::FULL_POWER * 60.0 * 60.0);
            return data::soc;
        }
    };

};