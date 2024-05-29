#pragma once

#include <Wire.h>
#include <Arduino.h>

#include <optional>

class ISL28022
{
private:
    /*Register Group*/
    enum REG : uint8_t
    {
        CONFIG,
        SHUNTVOLT,
        BUSVOLT,
        POWER,
        CURRENT,
        CALIB,
    };
    /*Regsiter Bits on CONFIG Group*/
    enum CONFIG : uint8_t
    {
        MODE0,
        MODE1,
        MODE2,
        SADC0,
        SADC1,
        SADC2,
        SADC3,
        BADC0,
        BADC1,
        BADC2,
        BADC3,
        PG0,
        PG1,
        BRNG0,
        BRNG1,
        RST
    };
    
    static constexpr double MAX_SHUNT_VOLTAGE = 320.0;
    static constexpr double SHUNT_RESISTANCE = 0.1;
    static constexpr double AMC1301_GAIN = 8.2;
    static constexpr double AMC1302_GAIN = 40;
    static constexpr double BUSVOL_RATIO = 38.906;
    static constexpr double SHUNTVOL_RATIO = 1 / (3.3 + 1 + 3.3);
    /* Create bit mask 
    return: uint16_t 
    argument: wake up bits min 0, max 15
    */
    template <typename... BITS>
    constexpr uint16_t make_mask(BITS... bits)
    {
        uint16_t ret{};
        for (uint8_t i : std::initializer_list<uint8_t>{bits...})
        {
            ret |= (0x01 << i);
        }
        return ret;
    }

    /* OverWrite Register Data
    return: None
    argument: RegisterGroupAddress, Data
    */
    void write_register(const uint8_t reg, const uint16_t mask)
    {
        hi2c_.beginTransmission(addr_);
        hi2c_.write(reg);
        hi2c_.write((mask >> 8) & 0xFF);
        hi2c_.write(mask & 0xFF);
        hi2c_.endTransmission();
    }
    /* Read Register Data
    return: Optional value(Error or int16_t)
    argument: Register Group Address, retry(millis)
    */

    std::optional<uint16_t> read_register(const uint8_t reg, const uint32_t wait = 10)
    {
        hi2c_.beginTransmission(addr_);
        hi2c_.write(reg);
        hi2c_.endTransmission();
        hi2c_.requestFrom(addr_, 2);
        uint32_t count{};
        while (!hi2c_.available() && count < wait)
        {
            delay(1);
            count++;
        }
        if (hi2c_.available())
        {
            byte first = hi2c_.read();
            byte second = hi2c_.read();
            return (first << 8 | second);
        }
        else
        {
            return std::nullopt;
        }
    }
    /* Calculate Constant ShuntVoltage Value
    On 15bit, +-320mv, Mode, Voltage(mV) = Reading Value * (320 / 2^15) 
    */
    constexpr double factor_shuntvoltage(){

        return MAX_SHUNT_VOLTAGE * AMC1301_GAIN * SHUNTVOL_RATIO / (std::numeric_limits<int16_t>::max() + 1);

    }
    /* Calculate Constant BusVoltage Value * BOARD_GAIN
    */
    constexpr double factor_busvoltage() {

        return 0.004 * BUSVOL_RATIO;

    }
    /* Calculate Constant Current Value
    Current(A) = Reading Value * BOARD_GAIN / (2^15) * (320) / ShuntRegistance */
    constexpr double factor_current()
    {

        return MAX_SHUNT_VOLTAGE * AMC1301_GAIN * SHUNTVOL_RATIO  / SHUNT_RESISTANCE /(std::numeric_limits<int16_t>::max() + 1);
    }
    /* Calculate Constant Power Value
    Power(W) = Reading Value * factor_current * factor_busvoltage * 5000 / 1000000

    */
    constexpr double factor_power()
    {
        return 97.65625 * 5000.0 * 2.0 / 1000000.0;
    }

    constexpr uint16_t calblation_value() {
        return 0.04096 / (factor_current() * SHUNT_RESISTANCE) * 1000;
    } 

    uint8_t addr_;
    TwoWire &hi2c_;

public:
    ISL28022(TwoWire &hi2c, uint8_t addr) : hi2c_(hi2c), addr_(addr)
    {
    }

    void begin()
    {
        write_register(REG::CONFIG, make_mask(CONFIG::RST));
        // busvol = 60v, shuntvol = +-320mv, resolution = 15bit, detect both bus and shunt vol
        write_register(REG::CONFIG, make_mask(CONFIG::BRNG0, CONFIG::BRNG0, CONFIG::BRNG1, CONFIG::PG1, CONFIG::PG0, CONFIG::BADC1, CONFIG::BADC0, CONFIG::SADC1, CONFIG::SADC0, CONFIG::MODE0, CONFIG::MODE1, CONFIG::MODE2));
        write_register(REG::CALIB, calblation_value());
        delay(10);
      
        Serial.println(("#factor_current " + std::to_string(factor_current())).c_str());
        Serial.println(("#factor_busvoltage " + std::to_string(factor_busvoltage())).c_str());
        Serial.println(("#factor_shuntvoltage " + std::to_string(factor_shuntvoltage())).c_str());
        Serial.println(("#factor_power " + std::to_string(factor_power())).c_str());

    }

    std::optional<float> GetShuntVoltage()
    {
        auto result = read_register(REG::SHUNTVOLT);
        if (result.has_value())
        {
            return static_cast<float>(result.value()) * factor_shuntvoltage();
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<float> GetBusVoltage() //OK
    {
        auto result = read_register(REG::BUSVOLT);
        if (result.has_value())
        {
            return (result.value() >> 2) * factor_busvoltage();
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<float> GetPower()
    {
        auto result = read_register(REG::POWER);
        if (result.has_value())
        {
            return result.value() * factor_power();
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<float> GetCurrent()
    {
        auto result = read_register(REG::CURRENT);
        if (result.has_value())              
        {
            return static_cast<int16_t>(result.value()) * factor_current();
        }
        else
        {
            return std::nullopt;
        }
    }
};