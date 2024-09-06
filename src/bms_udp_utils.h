#pragma once
#include <array>
#include <AsyncUDP.h>
#include <WiFi.h>
#include "params.h"
#include "ltc6811_driver.h"
namespace udp
{
    namespace protocol
    {
        namespace UDP_PACKET_ID
        {
            static constexpr uint8_t ID_LENGTH = 4;
            static constexpr const char *UDP_PACKET_BMS_BOARD_ID = "BBID";
            static constexpr const char *UDP_PACKET_BMS_STATUS_MAIN_IV = "STAT";
            static constexpr const char *UDP_PACKET_BMS_STATUS_CELLVOLTAGE = "CELL";
            static constexpr const char *UDP_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL = "CONF";
            static constexpr const char *UDP_PACKET_BMS_STATUS_TEMPERATURES = "TEMP";
            static constexpr const char *UDP_PACKET_BMS_STATUS_AUX_IV_SAFETY_WATCHDOG = "AUXW";
            static constexpr const char *UDP_PACKET_BMS_KEEP_ALIVE_SAFETY = "ALIV";
        };

        template <typename T, size_t S>
        std::array<uint8_t, S * sizeof(T) + sizeof(UDP_PACKET_ID::ID_LENGTH) * 2 + UDP_PACKET_ID::ID_LENGTH * 2> create_packet(const char *packet_id, uint8_t board_id, std::array<T, S> &data)
        {
            std::array<uint8_t, S * sizeof(T) + sizeof(UDP_PACKET_ID::ID_LENGTH) * 2 + UDP_PACKET_ID::ID_LENGTH * 2> packet = {};
            std::memcpy(packet.data(), packet_id, UDP_PACKET_ID::ID_LENGTH);
            packet[UDP_PACKET_ID::ID_LENGTH] = static_cast<uint8_t>(sizeof(board_id) + sizeof(data) + 2);
            std::memcpy(packet.data() + UDP_PACKET_ID::ID_LENGTH + sizeof(UDP_PACKET_ID::ID_LENGTH), board_id, UDP_PACKET_ID::ID_LENGTH);
            packet[UDP_PACKET_ID::ID_LENGTH * 2 + sizeof(UDP_PACKET_ID::ID_LENGTH)] = static_cast<uint8_t>(sizeof(board_id));
            std::memcpy(packet.data() + UDP_PACKET_ID::ID_LENGTH + sizeof(UDP_PACKET_ID::ID_LENGTH), data.data(), sizeof(data));
            return packet;
        }

    };

    namespace driver
    {
        AsyncUDP UDP;

        template <size_t S>
        void transmit(std::array<uint8_t, S> &data)
        {
            UDP.writeTo(data.data(), IPAdress(board::HOST_IP_ADDRESS.data()), 12351);
        }

        void setup()
        {
            WiFi.mode(WIFI_STA);
            WiFi.begin(board::SSID, board::PASSWORD);
        }

        bool report(uint32_t voltage, uint32_t current, ltc6811::data::CellVoltage &cell_data, ltc6811::data::Temperature &temp_data)
        {
            {
                std::array<uint32_t, 2> data = {voltage, current};
                transmit(protocol::create_packet(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_MAIN_IV, board::CAN_ID, data));
            }
            {
                std::array<uint32_t, 2> data = {cell_data.vol_range.first, cell_data.vol_range.second};
                transmit(protocol::create_packet(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_CELLVOLTAGE, board::CAN_ID, data));
            }
            {
                std::array<uint8_t, 8> data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0b00000100};
                transmit(protocol::create_packet(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL, board::CAN_ID, data));
            }
            {
                std::array<uint16_t, 4>
                    data = {
                        temp_data.battery_average,
                        temp_data.temp_range.first,
                        temp_data.pcb_average,
                        temp_data.temp_range.second};
                transmit(protocol::create_packet(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_TEMPERATURES, board::CAN_ID, data));
            }
            return true;
        }

    };
};