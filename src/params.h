#pragma once

#include <array>
#include <Arduino.h>

namespace board
{
    static constexpr uint32_t CAN_ID = 0x00;
    static constexpr uint32_t CAN_RX_PIN = 34;
    static constexpr uint32_t CAN_TX_PIN = 32;
    static constexpr uint32_t CAN_BITRATE = 125000;
    static constexpr uint32_t CHANE_LENGTH = 2;
    static constexpr uint32_t CELL_NUM_PER_IC = 12;
    static constexpr uint32_t THURMISTA_NUM_PER_IC = 5;
    static constexpr uint32_t REGISTER_BYTES = 8;
    static constexpr uint32_t WAKEUP_DELAY = 400;
    static constexpr uint32_t B_VALUE = 4550;
    static constexpr uint32_t THRMISTA_VOLTAGE = 30000.0;
    static constexpr std::array<uint8_t, 4> HOST_IP_ADDRESS = {192, 168, 3, 14};
    static constexpr const char *SSID = "ssid";
    static constexpr const char *PASSWORD = "password";

    using CELL_DATA = std::array<std::array<uint16_t, CELL_NUM_PER_IC>, CHANE_LENGTH>;
    using TEMP_DATA = std::array<std::array<int32_t, THURMISTA_NUM_PER_IC>, CHANE_LENGTH>;
    using VOL_DATA = float;

};
