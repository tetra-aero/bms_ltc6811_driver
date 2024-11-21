#pragma once
#include "Arduino.h"
namespace soc
{
    namespace param
    {
        constexpr double FULL_VOLTAGE = 44.4;
        constexpr double FULL_CAPACITY = 6000;                      // mAh
        constexpr double FULL_POWER = FULL_VOLTAGE * FULL_CAPACITY; // mWh
        constexpr uint32_t full_charge_notify = 0x4600;
    };
    namespace data
    {
        double soc = 100.0;
        double remain = soc * param::FULL_POWER * 60.0 * 60.0; // mW
        SemaphoreHandle_t soc_data_semaphore;
        void dbg()
        {
            Serial.println(("# SoC: " + std::to_string(static_cast<float>(soc)) + "[percent]").c_str());
        }
    };
    namespace driver
    {
        void setup()
        {
            data::soc_data_semaphore = xSemaphoreCreateBinary();
            xSemaphoreGive(data::soc_data_semaphore);
        }

        void full_recharge()
        {
            data::remain = 100 * param::FULL_POWER * 60.0 * 60.0;
            ;
        }

        double update_soc(float current_mA, float voltage, float delta_ms = 10.0)
        {
            data::remain -= voltage * current_mA * delta_ms / 1000.0;
            data::soc = data::remain / (param::FULL_POWER * 60.0 * 60.0);
            return data::soc;
        }
    };

};