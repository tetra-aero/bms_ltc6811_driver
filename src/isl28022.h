#pragma once

#include <Wire.h>
#include <Arduino.h>

#include <optional>

class ISL28022
{
private:
    enum REG : uint8_t
    {
        CONFIG,
        SHUNTVOLT,
        BUSVOLT,
        POWER,
        CURRENT,
        CALIB,
    };

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

    void write_register(const uint8_t reg, const uint16_t mask)
    {
        hi2c_.beginTransmission(addr_);
        hi2c_.write(reg);
        hi2c_.write((mask >> 8) & 0xFF);
        hi2c_.write(mask & 0xFF);
        hi2c_.endTransmission();
    }

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

    constexpr double factor_shuntvoltage(){
        return 320.0 / std::numeric_limits<int16_t>::max();
    }

    constexpr double factor_current()
    {
        return 320.0 / 5.0 / 32768.0;
    }

    constexpr double factor_power()
    {
        return 97.65625 * 5000.0 * 2.0 / 1000000.0;
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
        // busvol = 60v, shuntvol = +-250mv, resolution = 15bit, detect both bus and shunt vol
        write_register(REG::CONFIG, make_mask(CONFIG::BRNG0, CONFIG::BRNG0, CONFIG::BRNG1, CONFIG::PG1, CONFIG::PG0, CONFIG::BADC1, CONFIG::BADC0, CONFIG::SADC1, CONFIG::SADC0, CONFIG::MODE0, CONFIG::MODE1, CONFIG::MODE2));
        write_register(REG::CALIB, 0x1062);
    }

    std::optional<float> GetShuntVoltage()
    {
        auto result = read_register(REG::SHUNTVOLT);
        if (result.has_value())
        {
            return result.value() * factor_shuntvoltage();
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<float> GetBusVoltage()
    {
        auto result = read_register(REG::BUSVOLT);
        if (result.has_value())
        {
            return result.value() * 0.001;
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
            return result.value() * factor_current();
        }
        else
        {
            return std::nullopt;
        }
    }
};