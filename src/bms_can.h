#pragma once
#include <array>
#include <CAN.h>
#include "constant.h"
#include "ltc6811_params.h"

namespace can
{
    namespace protocol
    {
        enum class CAN_PACKET_ID : uint32_t
        {
            CAN_PACKET_BMS_STATUS_MAIN_IV = 30,
            CAN_PACKET_BMS_STATUS_CELLVOLTAGE,
            CAN_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL,
            CAN_PACKET_BMS_STATUS_TEMPERATURES,
            CAN_PACKET_BMS_STATUS_AUX_IV_SAFETY_WATCHDOG,
            CAN_PACKET_BMS_KEEP_ALIVE_SAFETY,
        };

        constexpr uint32_t create_packet_id(CAN_PACKET_ID packet_id, uint32_t board_id)
        {
            uint32_t id = (static_cast<uint32_t>(packet_id) << 8) | board_id;
            return id;
        }
    };

    namespace driver
    {
        template <class C, size_t S>
        void transmit(uint32_t packet_id, std::array<C, S> &data)
        {
            const uint8_t *buffer = reinterpret_cast<const uint8_t *>(data.data());
            const size_t size = sizeof(data) > 8 ? 8 : sizeof(data);
            CAN.beginExtendedPacket(packet_id);
            for (size_t i = 0; i < size; i++)
                CAN.write(buffer[i]);
            CAN.endPacket();
        }

        bool setup()
        {
            CAN.setPins(board::CAN_RX_PIN, board::CAN_TX_PIN);
            return CAN.begin(board::CAN_BITRATE * 2);
        }

        bool report(uint32_t voltage, uint32_t current, ltc6811::data::CellVoltage &cell_data, ltc6811::data::Temperature &temp_data)
        {
            {
                std::array<uint32_t, 2> data = {voltage, current};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_MAIN_IV, board::CAN_ID), data);
            }
            {
                std::array<uint32_t, 2> data = {cell_data.vol_range.first, cell_data.vol_range.second};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_CELLVOLTAGE, board::CAN_ID), data);
            }
            {
                std::array<uint8_t, 8> data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0b00000100};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL, board::CAN_ID), data);
            }
            {
                std::array<uint16_t, 4>
                    data = {
                        temp_data.battery_average,
                        temp_data.temp_range.first,
                        temp_data.pcb_average,
                        temp_data.temp_range.second};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_TEMPERATURES, board::CAN_ID), data);
            }
            return true;
        }
    };

};
