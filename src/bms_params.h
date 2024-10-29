#pragma once

#include <array>
#include <Arduino.h>

namespace board
{
    static constexpr uint32_t CAN_ID = 0x01;
    static constexpr uint32_t CHANE_LENGTH = 1;
    static constexpr uint32_t CAN_BITRATE = 500000;
    static constexpr uint32_t UART_BITRATE = 115200;
    static constexpr uint32_t CAN_RX_PIN = 34;
    static constexpr uint32_t CAN_TX_PIN = 32;

    static constexpr std::array<uint8_t, 4> HOST_IP_ADDRESS = {192, 168, 3, 11};
    static constexpr std::array<uint8_t, 4> DEVICE_IP_ADDRESS = {192, 168, 3, 14};
    static constexpr std::array<uint8_t, 4> GATEWAY_ADDRESS = {192, 168, 3, 1};
    static constexpr std::array<uint8_t, 4> SUBNET_MASK = {255, 255, 255, 0};
    static constexpr const char *SSID = "";
    static constexpr const char *PASSWORD = "";
    static constexpr uint32_t CELL_NUM_PER_IC = 12;
    static constexpr uint32_t THURMISTA_NUM_PER_IC = 5;
    using CELL_DATA = std::array<std::array<uint16_t, CELL_NUM_PER_IC>, CHANE_LENGTH>;
    using TEMP_DATA = std::array<std::array<int32_t, THURMISTA_NUM_PER_IC>, CHANE_LENGTH>;
    using VOL_DATA = float;
};
