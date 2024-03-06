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
        Wire.beginTransmission(addr_);
        Wire.write(reg);
        Wire.write((mask >> 8) & 0xFF);
        Wire.write(mask & 0xFF);
        Wire.endTransmission();
    }

    std::optional<float> read_register(const uint8_t reg, const uint32_t wait = 10)
    {
        Wire.beginTransmission(addr_);
        Wire.write(reg);
        Wire.endTransmission();
        Wire.requestFrom(addr_, 2);
        uint32_t count{};
        while (!Wire.available() && count < wait)
        {
            delay(1);
            count++;
        }
        if (Wire.available())
        {
            byte first = Wire.read();
            byte second = Wire.read();
            return (first << 8 | second);
        }
        else
        {
            return std::nullopt;
        }
    }

    uint8_t addr_;

public:
    ISL28022(uint8_t addr) : addr_(addr)
    {
    }

    void begin()
    {
        write_register(REG::CONFIG, make_mask(CONFIG::RST));
        // busvol = 60v, shuntvol = +-250mv, resolution = 15bit,sampling bus and shunt vol
        write_register(REG::CONFIG, make_mask(CONFIG::BRNG0,CONFIG::BRNG0,CONFIG::BRNG1,CONFIG::PG1,CONFIG::PG0,CONFIG::BADC1,CONFIG::BADC0,CONFIG::SADC1,CONFIG::SADC0,CONFIG::MODE0,CONFIG::MODE1,CONFIG::MODE2));
        write_register(REG::CALIB, make_mask());
    }

    std::optional<float> GetShuntVoltage()
    {
        auto result = read_register(REG::SHUNTVOLT);
        if (result.has_value())
        {
            return 
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
            return 
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
            return 
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
        }
        else
        {
            return std::nullopt;
        }
    }
};