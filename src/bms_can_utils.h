#pragma once

#include <array>
#include <CAN.h>
#include "bms_params.h"
#include "bms_spi_utils.h"
#include "bms_soc_utils.h"
namespace can
{
    bool init = false;
    namespace csnv700
    {
        namespace param
        {
            constexpr uint32_t packet_id = 0x3C4;

            enum class Error : uint8_t
            {
                FlashCRC = 0x48,
                AFEOVR,
                AFE,
                LUT,
                PMINLIM = 0x54,
                PMAXLIM = 0x55,
            };
        };

        namespace data
        {
            int32_t current;
            bool error;
            uint8_t error_type;

            void dbg()
            {
                Serial.println(("# CSNV700: Current : " + std::to_string(static_cast<float>(current)) + "[mA]").c_str());
            }
        };

        namespace utils
        {
            constexpr std::array<uint8_t, 256> lookup{
                0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
                0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
                0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
                0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
                0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
                0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
                0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
                0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
                0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
                0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
                0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
                0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
                0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
                0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
                0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
                0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
                0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
                0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
                0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
                0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
                0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
                0xfa, 0xfd, 0xf4, 0xf3};
            constexpr uint8_t CRC(const std::array<uint8_t, 7> &data)
            {
                uint8_t res = 0x00;
                for (int i = 0; i < data.size(); i++)
                {
                    res = lookup[(res ^ data[i])];
                }
                return res;
            }
        }

        namespace driver
        {
            struct Response
            {
                std::array<uint8_t, 7> data;
                uint8_t CRC;
                Response(uint8_t *buffer)
                {
                    std::copy(buffer, buffer + 7, data.begin());
                    CRC = buffer[7];
                }
                bool check_crc()
                {
                    return CRC == utils::CRC(data);
                }
                void parse()
                {
                    data::current = (data[3] | data[2] << 8 | data[1] << 16 | data[0] << 24) - 0x80000000;
                    data::error_type = data[4] >> 1;
                    data::error = data[4] & 0x01;
                }
            };

            void loop()
            {
                if (init & CAN.packetId() == param::packet_id && CAN.available() >= 8)
                {
                    uint8_t buffer[8];
                    CAN.readBytes(buffer, 8);
                    auto res = Response(buffer);
                    if (res.check_crc())
                    {
                        res.parse();
                    }
                }
            }
        };
    };

    namespace protocol
    {
        enum class CAN_PACKET_ID : uint32_t
        {
            CAN_PACKET_BMS_STATUS_MAIN_IV = 0x40,
            CAN_PACKET_BMS_STATUS_CELLVOLTAGE,
            CAN_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL,
            CAN_PACKET_BMS_STATUS_TEMPERATURES,
            CAN_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL,
            CAN_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL_REQUEST,
        };

        constexpr uint32_t create_packet_id(CAN_PACKET_ID packet_id, uint32_t board_id)
        {
            uint32_t id = (static_cast<uint32_t>(packet_id) << 8) | board_id;
            return id;
        }
    };

    namespace driver
    {
        QueueHandle_t can_message_queue;
        volatile SemaphoreHandle_t can_peripheral_semaphore;

        template <typename T, size_t S>
        void transmit(uint32_t packet_id, std::array<T, S> &data)
        {
            const uint8_t *buffer = reinterpret_cast<const uint8_t *>(data.data());
            const size_t size = sizeof(data) > 8 ? 8 : sizeof(data);
            xSemaphoreTake(can_peripheral_semaphore, portMAX_DELAY);
            CAN.beginExtendedPacket(packet_id);
            for (size_t i = 0; i < size; i++)
                CAN.write(buffer[i]);
            CAN.endPacket();
            xSemaphoreGive(can_peripheral_semaphore);
        }

        uint16_t create_cell_segment(spi::ltc6811::data::ic_id ic, spi::ltc6811::data::cell_id cell, uint16_t data)
        {
            return ((ic * board::CELL_NUM_PER_IC + cell) << 9) + (data);
        }

        void callback(int packetSize)
        {
            if (pdTRUE == xSemaphoreTakeFromISR(can_peripheral_semaphore, NULL))
            {
                if (CAN.packetId() == csnv700::param::packet_id & packetSize >= 8)
                {
                    uint8_t buffer[8];
                    CAN.readBytes(buffer, 8);
                    xQueueSendFromISR(can_message_queue, buffer, NULL);
                }
                else if (CAN.packetId() == soc::param::full_charge_notify + board::CAN_ID)
                {

                    if (pdTRUE == xSemaphoreTakeFromISR(soc::data::soc_data_semaphore, NULL))
                    {
                        Serial.println("aa");
                        soc::driver::full_recharge();
                        xSemaphoreGiveFromISR(soc::data::soc_data_semaphore, 0);
                    }
                }
                xSemaphoreGiveFromISR(can_peripheral_semaphore, 0);
            }
        }

        bool setup()
        {
            can_peripheral_semaphore = xSemaphoreCreateBinary();
            can_message_queue = xQueueCreate(20, sizeof(uint8_t) * 8);
            CAN.setPins(board::CAN_RX_PIN, board::CAN_TX_PIN);
            if (!CAN.begin(board::CAN_BITRATE * 2))
            {
                return true;
            }
            // CAN.filterExtended(((static_cast<uint32_t>(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL_REQUEST) << 8) + board::CAN_ID) ^ csnv700::param::packet_id);
            // CAN.filterExtended(csnv700::param::packet_id);
            CAN.onReceive(callback);
            xSemaphoreGive(can_peripheral_semaphore);
            init = true;
            return false;
        }

        bool report(uint32_t voltage, uint32_t current, float remain_f, float soc_f, spi::ltc6811::data::CellVoltage &cell_data, spi::ltc6811::data::Temperature &temp_data)
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
                uint16_t remain = static_cast<uint16_t>(remain_f);
                uint8_t soc = static_cast<uint8_t>(soc_f);
                std::array<uint8_t, 8>
                    data = {0x00, 0x00, (remain & 0xFF), (remain & 0xFF00) >> 8, soc, 0x00, 0x00, 0b00000100};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL, board::CAN_ID), data);
            }
            {
                std::array<int16_t, 4>
                    data = {
                        static_cast<int16_t>(temp_data.battery_average / 1000),
                        static_cast<int16_t>(temp_data.temp_range_battery.second / 1000),
                        static_cast<int16_t>(temp_data.pcb_average / 1000),
                        static_cast<int16_t>(temp_data.temp_range_pcb.second / 1000)};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_TEMPERATURES, board::CAN_ID), data);
            }
            std::array<uint16_t, 4> data;
            for (size_t i = 0; i < cell_data.vol.size(); i++)
            {
                data = {create_cell_segment(i, 0, cell_data.vol[i][0] / 100), create_cell_segment(i, 1, cell_data.vol[i][1] / 100), create_cell_segment(i, 2, cell_data.vol[i][2] / 100), create_cell_segment(i, 3, cell_data.vol[i][3] / 100)};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL, board::CAN_ID), data);
                data = {create_cell_segment(i, 4, cell_data.vol[i][4] / 100), create_cell_segment(i, 5, cell_data.vol[i][5] / 100), create_cell_segment(i, 6, cell_data.vol[i][6] / 100), create_cell_segment(i, 7, cell_data.vol[i][7] / 100)};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL, board::CAN_ID), data);
                data = {create_cell_segment(i, 8, cell_data.vol[i][8] / 100), create_cell_segment(i, 9, cell_data.vol[i][9] / 100), create_cell_segment(i, 10, cell_data.vol[i][10] / 100), create_cell_segment(i, 11, cell_data.vol[i][11] / 100)};
                transmit(protocol::create_packet_id(protocol::CAN_PACKET_ID::CAN_PACKET_BMS_STATUS_CELLVOLTAGE_DETAIL, board::CAN_ID), data);
            }
            return true;
        }
    };

};
