#pragma once
#include <array>
#include <cstring>
#include <AsyncUDP.h>
#include <WiFi.h>
#include "bms_params.h"
#include "bms_ltc6811_driver.h"
namespace udp
{
    namespace protocol
    {

        enum class UDP_PACKET_ID : uint32_t
        {
            UDP_PACKET_BMS_STATUS_MAIN_IV = 0x40,
            UDP_PACKET_BMS_STATUS_CELLVOLTAGE,
            UDP_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL,
            UDP_PACKET_BMS_STATUS_TEMPERATURES,
            UDP_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL,
            UDP_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL_REQUEST,
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
        bool request = false;

        template <typename T, size_t S>
        void transmit(uint32_t packet_id, std::array<T, S> &data)
        {
            std::array<uint8_t, sizeof(packet_id) + S * sizeof(T)> buffer;
            std::memcpy(buffer.data(), &packet_id, sizeof(packet_id));
            std::memcpy(buffer.data() + sizeof(packet_id), data.data(), sizeof(data));
            UDP.writeTo(buffer.data(), buffer.size(), IPAddress(board::HOST_IP_ADDRESS.data()), 12351);
        }

        uint16_t create_cell_segment(ltc6811::data::ic_id ic, ltc6811::data::cell_id cell, uint16_t data)
        {
            return ((ic * board::CELL_NUM_PER_IC + cell) << 9) + (data);
        }

        void response_cellvol(AsyncUDPPacket &packet)
        {
            uint32_t request_id = *reinterpret_cast<uint32_t *>(packet.data());
            if (request_id == (protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL_REQUEST, board::CAN_ID)))
            {
                request = true;
            }
        }

        void setup()
        {
            WiFi.config(IPAddress(board::DEVICE_IP_ADDRESS.data()), IPAddress(board::GATEWAY_ADDRESS.data()), IPAddress(board::SUBNET_MASK.data()));
            WiFi.mode(WIFI_STA);
            WiFi.begin(board::SSID, board::PASSWORD);
            if (UDP.listen(IP_ANY_TYPE, 12351))
            {
                UDP.onPacket(response_cellvol);
            }
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
                std::array<int16_t, 4>
                    data = {
                        static_cast<int16_t>(temp_data.battery_average / 1000),
                        static_cast<int16_t>(temp_data.temp_range.second / 1000),
                        static_cast<int16_t>(temp_data.pcb_average / 1000),
                        static_cast<int16_t>(temp_data.temp_range.second / 1000)};
                transmit(protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_TEMPERATURES, board::CAN_ID), data);
            }
            if (request)
            {
                std::array<uint16_t, 4> data;
                for (size_t i = 0; i < ltc6811::data::cell_data.vol.size(); i++)
                {
                    data = {create_cell_segment(i, 0, ltc6811::data::cell_data.vol[i][0] / 100), create_cell_segment(i, 1, ltc6811::data::cell_data.vol[i][1] / 100), create_cell_segment(i, 2, ltc6811::data::cell_data.vol[i][2] / 100), create_cell_segment(i, 3, ltc6811::data::cell_data.vol[i][3] / 100)};
                    transmit(protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL, board::CAN_ID), data);
                    data = {create_cell_segment(i, 4, ltc6811::data::cell_data.vol[i][4] / 100), create_cell_segment(i, 5, ltc6811::data::cell_data.vol[i][5] / 100), create_cell_segment(i, 6, ltc6811::data::cell_data.vol[i][6] / 100), create_cell_segment(i, 7, ltc6811::data::cell_data.vol[i][7] / 100)};
                    transmit(protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL, board::CAN_ID), data);
                    data = {create_cell_segment(i, 8, ltc6811::data::cell_data.vol[i][8] / 100), create_cell_segment(i, 9, ltc6811::data::cell_data.vol[i][9] / 100), create_cell_segment(i, 10, ltc6811::data::cell_data.vol[i][10] / 100), create_cell_segment(i, 11, ltc6811::data::cell_data.vol[i][11] / 100)};
                    transmit(protocol::create_packet_id(protocol::UDP_PACKET_ID::UDP_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL, board::CAN_ID), data);
                }
                request = false;
            }
            return true;
        }

    };
};