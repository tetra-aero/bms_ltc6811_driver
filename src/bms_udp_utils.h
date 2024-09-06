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

        enum class UDP_PACKET_ID : uint32_t
        {
            UDP_PACKET_BMS_STATUS_MAIN_IV = 30,
            UDP_PACKET_BMS_STATUS_CELLVOLTAGE,
            UDP_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL,
            UDP_PACKET_BMS_STATUS_TEMPERATURES,
            UDP_PACKET_BMS_STATUS_AUX_IV_SAFETY_WATCHDOG,
            UDP_PACKET_BMS_KEEP_ALIVE_SAFETY,
        };

        constexpr uint32_t create_packet_id(UDP_PACKET_ID packet_id, uint32_t board_id)
        {
            uint32_t id = (static_cast<uint32_t>(packet_id) << 8) | board_id;
            return id;
        }

    };

    namespace driver
    {
        AsyncUDP UDP;

        template <typename T, size_t S>
        void transmit(uint32_t packet_id, std::array<T, S> &data)
        {
            const uint8_t *buffer = reinterpret_cast<const uint8_t *>(data.data());
            const size_t size = sizeof(data) > 8 ? 8 : sizeof(data);
            UDP.writeTo(buffer, size, IPAddress(board::HOST_IP_ADDRESS.data()), 12351);
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
                transmit(protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_MAIN_IV, board::CAN_ID), data);
            }
            {
                std::array<uint32_t, 2> data = {cell_data.vol_range.first, cell_data.vol_range.second};
                transmit(protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_CELLVOLTAGE, board::CAN_ID), data);
            }
            {
                std::array<uint8_t, 8> data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0b00000100};
                transmit(protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL, board::CAN_ID), data);
            }
            {
                std::array<uint16_t, 4>
                    data = {
                        temp_data.battery_average,
                        temp_data.temp_range.first,
                        temp_data.pcb_average,
                        temp_data.temp_range.second};
                transmit(protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_TEMPERATURES, board::CAN_ID), data);
            }
            return true;
        }

    };
};