#pragma once 

#include <Wire.h>
#include <Arduino.h>

class ISL28022 {
    private:
    static constexpr uint8_t SHUNTVOLT = 0x01;
    static constexpr uint8_t BUSVOLT = 0x02;
    static constexpr uint8_t CURRENT = 0x04;
    static constexpr uint8_t POWER = 0x03; 

    enum REG : uint8_t {
        SHUNTVOLT = 0x01,
        BUSVOLT,
        CURRENT,
        POWER
    };

    uint8_t addr_;

    public:
    ISL28022(uint8_t addr) : addr_(addr){

    }

    void begin(){
        Wire.beginTransmission(addr_);

    }

};