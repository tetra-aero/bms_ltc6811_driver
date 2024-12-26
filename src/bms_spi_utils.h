#pragma once
#include "bms_params.h"
#include "SPI.h"
#include <array>
#include <algorithm>
#include <cmath>
namespace spi
{
    bool init = false;
    namespace ltc6811
    {
        namespace param
        {
            enum class Mode : uint8_t
            {
                Fast = 1,
                Normal,
                Filtered,
            };

            enum class Dcp : uint8_t
            {
                Disable,
                Enable,
            };

            enum class Duty : uint8_t
            {
                Ratio_1_16,
                Ratio_2_16,
                Ratio_3_16,
                Ratio_4_16,
                Ratio_5_16,
                Ratio_6_16,
                Ratio_7_16,
                Ratio_8_16,
                Ratio_9_16,
                Ratio_10_16,
                Ratio_11_16,
                Ratio_12_16,
                Ratio_13_16,
                Ratio_14_16,
                Ratio_15_16,
                Ratio_16_16,
            };
            constexpr uint32_t T_REF_MAX = 4400;
            constexpr uint32_t T_CYCLE_FAST_MAX = 1185;
            static constexpr Dcp DISCHARGE_PERMISSION = Dcp::Enable;
            static constexpr Duty DUTY_RATIO = Duty::Ratio_1_16;
            static constexpr Mode DETECTION_MODE = Mode::Normal;

            static constexpr double TEMP_LIMIT = 50.0;
            static constexpr double VOL_DIF_TOL_LIMIT = 50;
            static constexpr double VOL_DIF_ABS_LIMIT = 20;

            static constexpr uint32_t REGISTER_BYTES = 8;
            static constexpr uint32_t WAKEUP_DELAY = 400;
            static constexpr double B_VALUE = 4550.0;
            static constexpr double THRMISTA_VOLTAGE = 30000.0;
            static constexpr std::initializer_list<uint8_t> PCB_THRMISTA_ID = {0};
            static constexpr std::initializer_list<uint8_t> BATTERY_THRMISTA_ID = {1, 2, 3, 4};
        };

        namespace data
        {
            using ic_id = uint8_t;
            using cell_id = uint8_t;
            using thrm_id = uint8_t;
            SemaphoreHandle_t ltc6811_data_semaphore;

            struct CellVoltage
            {
                uint64_t sum{};
                uint64_t average{};
                board::CELL_DATA vol;
                std::pair<uint16_t, uint16_t> vol_range{std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::min()};
                std::pair<ic_id, cell_id> min_id{0xFF, 0xFF};
                std::pair<ic_id, cell_id> max_id{0xFF, 0xFF};
                void clear()
                {
                    sum = 0;
                    vol_range = {std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::min()};
                }
                void update(ic_id ic, cell_id cell, uint16_t voltage)
                {
                    vol[ic][cell] = voltage;
                    sum += voltage;
                    if (vol_range.first > voltage)
                    {
                        vol_range.first = voltage;
                        min_id = {ic, cell};
                    }
                    if (voltage > vol_range.second)
                    {
                        vol_range.second = voltage;
                        max_id = {ic, cell};
                    }
                }
                void update_statistic()
                {
                    average = sum / (board::CHANE_LENGTH * board::CELL_NUM_PER_IC);
                }
            } cell_data;

            struct Temperature
            {
                board::TEMP_DATA temp;
                std::array<bool, board::CHANE_LENGTH> over;
                int64_t battery_average;
                int64_t pcb_average;
                std::array<int32_t, board::CHANE_LENGTH> vref2;
                std::pair<int32_t, int32_t> temp_range{std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()};
                std::pair<int32_t, int32_t> temp_range_pcb{std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()};
                std::pair<int32_t, int32_t> temp_range_battery{std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()};
                std::pair<ic_id, thrm_id> min_id{0xFF, 0xFF};
                std::pair<ic_id, thrm_id> max_id{0xFF, 0xFF};

                void clear()
                {
                    temp_range = {std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()};
                    temp_range_pcb = {std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()};
                    temp_range_battery = {std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()};
                    for (auto &x : over)
                    {
                        x = false;
                    }
                }

                void update(ic_id ic, thrm_id cell, int32_t celsius)
                {
                    temp[ic][cell] = celsius;
                    if (celsius > (param::TEMP_LIMIT * 1000))
                    {
                        over[ic] = true;
                    }
                    if (temp_range.first > celsius)
                    {
                        temp_range.first = celsius;
                        min_id = {ic, cell};
                    }
                    if (celsius > temp_range.second)
                    {
                        temp_range.second = celsius;
                        max_id = {ic, cell};
                    }
                }

                void update_statistic()
                {
                    pcb_average = 0;
                    battery_average = 0;

                    for (const auto index : param::PCB_THRMISTA_ID)
                    {
                        for (auto &x : temp)
                        {
                            pcb_average += x[index];
                            if (temp_range_pcb.first > x[index])
                            {
                                temp_range_pcb.first = x[index];
                            }
                            if (x[index] > temp_range_pcb.second)
                            {
                                temp_range_pcb.second = x[index];
                            }
                        }
                    }

                    for (const auto index : param::BATTERY_THRMISTA_ID)
                    {
                        for (auto &x : temp)
                        {
                            battery_average += x[index];
                            if (temp_range_battery.first > x[index])
                            {
                                temp_range_battery.first = x[index];
                            }
                            if (x[index] > temp_range_battery.second)
                            {
                                temp_range_battery.second = x[index];
                            }
                        }
                    }
                    pcb_average /= (board::CHANE_LENGTH * param::PCB_THRMISTA_ID.size());
                    battery_average /= (board::CHANE_LENGTH * param::BATTERY_THRMISTA_ID.size());
                }
            } temp_data;

            struct Status
            {
                struct Data
                {
                    float sumOfCells;
                    float internalDieTemp;
                    float vdigital;
                    float vanalog;
                };
                std::array<Data, board::CHANE_LENGTH> data;
            } status;

            struct Pwm
            {
                std::array<std::array<uint8_t, board::CELL_NUM_PER_IC>, board::CHANE_LENGTH> data;
            } pwm;

            struct Config
            {
                std::array<std::array<bool, board::CELL_NUM_PER_IC>, board::CHANE_LENGTH> data;
            } config;

            void dbg()
            {
                Serial.println(("# ltc6811: Temp: Min : " + std::to_string(static_cast<float>(temp_data.temp_range.first) / 1000)).c_str());
                Serial.println(("# ltc6811: Temp: Max : " + std::to_string(static_cast<float>(temp_data.temp_range.second) / 1000)).c_str());
                Serial.println(("# ltc6811: Temp: pcbAverage : " + std::to_string(static_cast<float>(temp_data.pcb_average) / 1000)).c_str());
                Serial.println(("# ltc6811: Temp: battAverage : " + std::to_string(static_cast<float>(temp_data.battery_average) / 1000)).c_str());
                Serial.println(("# ltc6811: Vol: Min : " + std::to_string(static_cast<float>(cell_data.vol_range.first) / 10000)).c_str());
                Serial.println(("# ltc6811: Vol: Max : " + std::to_string(static_cast<float>(cell_data.vol_range.second) / 10000)).c_str());
                Serial.println(("# ltc6811: Vol: Ave : " + std::to_string(static_cast<float>(cell_data.average) / 10000)).c_str());
                Serial.println(("# ltc6811: Vol: Sum : " + std::to_string(static_cast<float>(cell_data.sum) / 10000)).c_str());
                for (size_t j = 0; j < board::CHANE_LENGTH; j++)
                {
                    Serial.print(("# ltc6811: DisCharge : IC" + std::to_string(j) + " Cell :").c_str());
                    for (size_t i = 0; i < board::CELL_NUM_PER_IC; i++)
                    {
                        Serial.print((std::to_string(i) + "-").c_str());
                        if (config.data[j][i])
                        {
                            Serial.print("RUN");
                        }
                        else
                        {
                            Serial.print("STOP");
                        }
                        Serial.print(" ");
                    }
                    Serial.println();
                }
                for (size_t j = 0; j < board::CHANE_LENGTH; j++)
                {
                    Serial.print(("# ltc6811: Pwm Ratio: IC" + std::to_string(j) + " Cell :").c_str());
                    for (size_t i = 0; i < board::CELL_NUM_PER_IC; i++)
                    {
                        Serial.print((std::to_string(i) + "-" + std::to_string(static_cast<float>(pwm.data[j][i] + 1) * 1 / 16) + " ").c_str());
                    }
                    Serial.println();
                }
                for (size_t j = 0; j < board::CHANE_LENGTH; j++)
                {
                    Serial.print(("# ltc6811: Cell Vol: IC" + std::to_string(j) + " Cell :").c_str());
                    for (size_t i = 0; i < board::CELL_NUM_PER_IC; i++)
                    {
                        Serial.print((std::to_string(i) + "-" + std::to_string(static_cast<float>(cell_data.vol[j][i]) / 10000) + "V ").c_str());
                    }
                    Serial.println();
                }
                for (size_t j = 0; j < board::CHANE_LENGTH; j++)
                {
                    Serial.print(("# ltc6811: Temp: IC" + std::to_string(j) + " Thermista :").c_str());
                    for (size_t i = 0; i < board::THURMISTA_NUM_PER_IC; i++)
                    {
                        Serial.print((std::to_string(i) + "-" + std::to_string(static_cast<float>(temp_data.temp[j][i]) / 1000) + "deg ").c_str());
                    }
                    if (temp_data.over[j])
                        Serial.print(" OVERTEMP ");
                    Serial.println();
                    Serial.println(("# ltc6811: Temp: IC" + std::to_string(j) + " Thermista Voltage : " + std::to_string(static_cast<float>(temp_data.vref2[j]) / 10000) + "V").c_str());
                }
            }

        };

        namespace utils
        {
            constexpr std::array<uint16_t, 256> lookup{
                0x0000, 0xc599, 0xceab, 0x0b32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac, 0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa,
                0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d,
                0x5b2e, 0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4,
                0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544, 0x02be, 0xc727, 0xcc15, 0x098c, 0xda71, 0x1fe8, 0x14da, 0xd143,
                0xf3c5, 0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x07c2, 0xc25b, 0xc969, 0x0cf0, 0xdf0d, 0x1a94, 0x11a6, 0xd43f,
                0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8,
                0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba, 0x4a88, 0x8f11,
                0x057c, 0xc0e5, 0xcbd7, 0x0e4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286,
                0xa213, 0x678a, 0x6cb8, 0xa921, 0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9,
                0x0f84, 0xca1d, 0xc12f, 0x04b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e,
                0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 0x2ac0, 0x0d3a, 0xc8a3, 0xc391, 0x0608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7,
                0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9, 0x7350,
                0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c,
                0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x0846, 0xcddf, 0xc6ed, 0x0374, 0xd089, 0x1510, 0x1e22, 0xdbbb,
                0x0af8, 0xcf61, 0xc453, 0x01ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02,
                0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095};

            template <typename T, size_t S>
            constexpr uint16_t CRC(const std::array<T, S> &arr, size_t size = S * sizeof(T))
            {
                uint16_t res = 0x10;
                uint16_t idx = 0x00;
                auto data = reinterpret_cast<uint8_t const *>(arr.data());

                for (size_t i = 0; i < size; ++i)
                {
                    idx = (res >> 7 ^ data[i]) & 0xFF;
                    res = res << 8 ^ lookup[idx];
                }
                res = res << 1;
                return res;
            }

            constexpr std::array<uint8_t, 2> endian8(uint16_t crc)
            {
                return {static_cast<uint8_t>((crc >> 8) & 0xFF), static_cast<uint8_t>(crc & 0xFF)};
            }

            constexpr uint16_t endian16(uint16_t crc)
            {
                return ((crc >> 8) & 0x00FF) | ((crc << 8) & 0xFF00);
            }

            void wakeup_port(SPIClass &spi, uint8_t gpio)
            {
                for (size_t i = 0; i < board::CHANE_LENGTH; ++i)
                {
                    digitalWrite(gpio, LOW);
                    spi.transfer(0xFF);
                    digitalWrite(gpio, HIGH);
                }
            }

            void wakeup(SPIClass &spi, uint8_t gpio)
            {
                for (size_t i = 0; i < board::CHANE_LENGTH; ++i)
                {
                    digitalWrite(gpio, LOW);
                    delayMicroseconds(param::WAKEUP_DELAY);
                    digitalWrite(gpio, HIGH);
                    delayMicroseconds(10);
                }
            }

            int32_t Bvalue(uint16_t NTC_voltage)
            {
                double R = NTC_voltage / (param::THRMISTA_VOLTAGE - NTC_voltage);
                double TempInv = ((1 / (273.15 + 25)) + (std::log(R) / param::B_VALUE));
                return static_cast<int32_t>(((1 / TempInv) * 1000 - 273150));
            }

        };

        namespace registers
        {
            using COMMAND = std::array<uint8_t, 2>;
            using COMMANDCRC = std::array<uint8_t, 4>;
            template <typename T>
            using RESPONSE = std::array<T, 6 / sizeof(T)>;
            constexpr COMMAND MODE_BITS = {
                (static_cast<uint8_t>(param::DETECTION_MODE) & 0x02) >> 1,
                (static_cast<uint8_t>(param::DETECTION_MODE) & 0x01) << 7};
            constexpr COMMAND START_CONV_CV = {0x02 | MODE_BITS[0], MODE_BITS[1] | 0x60 | (static_cast<uint8_t>(param::DISCHARGE_PERMISSION) << 4)};
            constexpr COMMAND START_CONV_AX = {0x04 | MODE_BITS[0], MODE_BITS[1] | 0x60};
            constexpr COMMAND START_CONV_STAT = {0x04 | MODE_BITS[0], MODE_BITS[1] | 0x68};
            constexpr COMMAND WRITE_CONFIG_A = {0x00, 0x01};
            constexpr COMMAND READ_CONFIG_A = {0x00, 0x02};
            constexpr COMMAND READ_CELL_A = {0x00, 0x04};
            constexpr COMMAND READ_CELL_B = {0x00, 0x06};
            constexpr COMMAND READ_CELL_C = {0x00, 0x08};
            constexpr COMMAND READ_CELL_D = {0x00, 0x0A};
            constexpr COMMAND READ_TEMP_A = {0x00, 0x0C};
            constexpr COMMAND READ_TEMP_B = {0x00, 0x0E};
            constexpr COMMAND READ_STATUS_A = {0x00, 0x10};
            constexpr COMMAND READ_STATUS_B = {0x00, 0x12};
            constexpr COMMAND WRITE_PWM = {0x00, 0x20};
            constexpr COMMAND READ_PWM = {0x00, 0x22};

            constexpr COMMANDCRC create_command_crc(COMMAND command)
            {
                return {command[0], command[1], utils::endian8(utils::CRC(command, 2))[0], utils::endian8(utils::CRC(command, 2))[1]};
            }

            template <typename T>
            struct Response
            {
                std::array<T, (param::REGISTER_BYTES - sizeof(uint16_t)) / sizeof(T)> data;
                uint16_t CRC;
            };

            template <typename T>
            struct Responses
            {
                std::array<Response<T>, board::CHANE_LENGTH> data;
                bool check_crc()
                {
                    for (Response<T> &x : data)
                    {
                        if (utils::endian16(x.CRC) != utils::CRC(x.data))
                            return false;
                    }
                    return true;
                }
            };

            struct Pwm
            {
                void parse(Responses<uint8_t> &data)
                {
                    data::ic_id ic{};
                    data::cell_id cell{};
                    for (const Response<uint8_t> &res : data.data)
                    {
                        for (const uint8_t &x : res.data)
                        {
                            data::pwm.data[ic][cell++] = x & 0x0F;
                            data::pwm.data[ic][cell++] = (x >> 4) & 0x0F;
                        }
                        cell = 0;
                        ic++;
                    }
                }

                void set(Responses<uint8_t> &messages, data::Pwm &data)
                {
                    data::ic_id ic{0};
                    for (auto &message : messages.data)
                    {
                        uint8_t pwm_reg_val = static_cast<uint8_t>((data.data[ic][1]) << 4 | data.data[ic][0]);
                        message.data = {pwm_reg_val, pwm_reg_val, pwm_reg_val, pwm_reg_val, pwm_reg_val, pwm_reg_val};
                        message.CRC = utils::endian16(utils::CRC(message.data));
                        ic++;
                    }
                }
            };

            struct StatusRegisterA
            {
                void parse(Responses<uint16_t> &data)
                {
                    data::ic_id ic{};
                    for (const Response<uint16_t> &res : data.data)
                    {
                        uint16_t sc = res.data[0];
                        uint16_t itmp = res.data[1];
                        uint16_t va = res.data[2];
                        data::status.data[ic].sumOfCells = static_cast<float>(sc) / 1e4;
                        data::status.data[ic].internalDieTemp = (static_cast<float>(itmp) / 10 / 7.5) - 273.15;
                        data::status.data[ic].vanalog = static_cast<float>(va) / 1e4;
                        ic++;
                    }
                }
            };

            struct StatusRegisterB
            {
                void parse(Responses<uint16_t> &data)
                {
                    data::ic_id ic{};
                    for (const Response<uint16_t> &res : data.data)
                    {
                        uint16_t vd = res.data[0];
                        data::status.data[ic].vdigital = static_cast<float>(vd) / 1e4;
                        ic++;
                    }
                }
            };

            struct CellVoltageA
            {
                void parse(Responses<uint16_t> &data)
                {
                    data::ic_id ic{};
                    for (const Response<uint16_t> &res : data.data)
                    {
                        data::cell_id cell = 0;
                        for (const uint16_t vol : res.data)
                        {
                            data::cell_data.update(ic, cell++, vol);
                        }
                        ic++;
                    }
                }
            };

            struct CellVoltageB
            {
                void parse(Responses<uint16_t> &data)
                {
                    data::ic_id ic{};

                    for (const Response<uint16_t> &res : data.data)
                    {
                        data::cell_id cell = 3;
                        for (const uint16_t vol : res.data)
                        {
                            data::cell_data.update(ic, cell++, vol);
                        }
                        ic++;
                    }
                }
            };

            struct CellVoltageC
            {
                void parse(Responses<uint16_t> &data)
                {
                    data::ic_id ic{};

                    for (const Response<uint16_t> &res : data.data)
                    {
                        data::cell_id cell = 6;
                        for (const uint16_t vol : res.data)
                        {
                            data::cell_data.update(ic, cell++, vol);
                        }
                        ic++;
                    }
                }
            };

            struct CellVoltageD
            {
                void parse(Responses<uint16_t> &data)
                {
                    data::ic_id ic{};

                    for (const Response<uint16_t> &res : data.data)
                    {
                        data::cell_id cell = 9;
                        for (const uint16_t vol : res.data)
                        {
                            data::cell_data.update(ic, cell++, vol);
                        }
                        ic++;
                    }
                }
            };

            struct TempA
            {
                void parse(Responses<uint16_t> &data)
                {
                    data::ic_id ic{};

                    for (const Response<uint16_t> &res : data.data)
                    {
                        data::thrm_id thrm = 0;
                        for (const uint16_t temp : res.data)
                        {
                            data::temp_data.update(ic, thrm++, utils::Bvalue(temp));
                        }
                        ic++;
                    }
                }
            };

            struct TempB
            {
                void parse(Responses<uint16_t> &data)
                {
                    data::ic_id ic{};

                    for (const Response<uint16_t> &res : data.data)
                    {
                        data::thrm_id thrm = 3;
                        for (const uint16_t temp : res.data)
                        {

                            if (thrm == 5)
                            {
                                data::temp_data.vref2[ic] = temp;
                            }
                            else
                            {
                                data::temp_data.update(ic, thrm++, utils::Bvalue(temp));
                            }
                        }
                        ic++;
                    }
                }
            };

            struct Config
            {
                uint8_t create_bitmask(const std::array<bool, board::CELL_NUM_PER_IC> &dccx, uint8_t begin, uint8_t end)
                {
                    uint8_t ret = 0;
                    for (size_t i = begin; i <= end; i++)
                    {
                        ret |= (dccx[i] ? (0x01 << (i - begin)) : 0x00);
                    }
                    return ret;
                }

                void parse(Responses<uint8_t> &data)
                {
                    data::ic_id ic{};
                    for (const Response<uint8_t> &res : data.data)
                    {
                        uint16_t dcc = (static_cast<uint16_t>(res.data[5]) << 8) | res.data[4];
                        for (size_t i = 0; i < board::CELL_NUM_PER_IC; i++)
                        {
                            data::config.data[ic][i] = (dcc >> i) & 0x01;
                        }
                        ic++;
                    }
                }

                void set(Responses<uint8_t> &messages, const data::Config &dcc)
                {
                    data::ic_id ic{0};
                    for (auto &message : messages.data)
                    {
                        message.data[0] = 0xFC;
                        message.data[4] = create_bitmask(dcc.data[ic], 0, 7);
                        message.data[5] = create_bitmask(dcc.data[ic], 8, 11);
                        message.CRC = utils::endian16(utils::CRC(message.data));
                        ic++;
                    }
                }
            };

            template <typename T, class C>
            struct ReadRequest
            {
                COMMANDCRC cmd;
                Responses<T> res;
                C parser;
                constexpr ReadRequest(COMMANDCRC &&arr) : cmd(std::move(arr))
                {
                }
                std::optional<std::reference_wrapper<Responses<T>>> request(SPIClass &spi, uint8_t gpio)
                {
                    uint8_t *buffer = reinterpret_cast<uint8_t *>(res.data.data());
                    utils::wakeup_port(spi, gpio);
                    digitalWrite(gpio, LOW);
                    spi.writeBytes(cmd.data(), sizeof(cmd));
                    for (size_t i = 0; i < board::CHANE_LENGTH * param::REGISTER_BYTES; i++)
                    {
                        buffer[i] = spi.transfer(0xFF);
                    }
                    digitalWrite(gpio, HIGH);
                    if (!res.check_crc())
                        return std::nullopt;
                    return res;
                }
                void parse()
                {
                    parser.parse(res);
                }
            };

            template <typename T, class C>
            struct WriteRequest
            {
                COMMANDCRC cmd;
                Responses<T> message;
                C setter;

                constexpr WriteRequest(COMMANDCRC &&arr) : cmd(std::move(arr))
                {
                }

                std::optional<bool> request(SPIClass &spi, uint8_t gpio)
                {
                    utils::wakeup_port(spi, gpio);
                    digitalWrite(gpio, LOW);
                    spi.writeBytes(cmd.data(), sizeof(cmd));
                    uint8_t *buffer = reinterpret_cast<uint8_t *>(message.data.data());
                    spi.writeBytes(buffer, sizeof(message.data));
                    digitalWrite(gpio, HIGH);
                    return true;
                }

                template <class D>
                void set(D &data)
                {
                    setter.set(message, data);
                }
            };

            struct Request
            {
                COMMANDCRC cmd;

                constexpr Request(COMMANDCRC &&arr) : cmd(std::move(arr))
                {
                }

                std::optional<bool> request(SPIClass &spi, uint8_t gpio)
                {
                    utils::wakeup_port(spi, gpio);
                    digitalWrite(gpio, LOW);
                    delayMicroseconds(500);
                    spi.writeBytes(cmd.data(), sizeof(cmd));
                    delayMicroseconds(500);
                    digitalWrite(gpio, HIGH);
                    delayMicroseconds((param::T_REF_MAX + param::T_CYCLE_FAST_MAX));
                    return true;
                }
            };

            Request req_start_conv_cv(create_command_crc(START_CONV_CV));
            Request req_start_conv_ax(create_command_crc(START_CONV_AX));
            Request req_start_conv_stat(create_command_crc(START_CONV_STAT));
            WriteRequest<uint8_t, Config> req_write_config_a(create_command_crc(WRITE_CONFIG_A));
            ReadRequest<uint8_t, Config> req_read_config_a(create_command_crc(READ_CONFIG_A));
            WriteRequest<uint8_t, Pwm> req_write_pwm(create_command_crc(WRITE_PWM));
            ReadRequest<uint8_t, Pwm> req_read_pwm(create_command_crc(READ_PWM));
            ReadRequest<uint16_t, CellVoltageA> req_read_cell_a(create_command_crc(READ_CELL_A));
            ReadRequest<uint16_t, CellVoltageB> req_read_cell_b(create_command_crc(READ_CELL_B));
            ReadRequest<uint16_t, CellVoltageC> req_read_cell_c(create_command_crc(READ_CELL_C));
            ReadRequest<uint16_t, CellVoltageD> req_read_cell_d(create_command_crc(READ_CELL_D));
            ReadRequest<uint16_t, TempA> req_read_temp_a(create_command_crc(READ_TEMP_A));
            ReadRequest<uint16_t, TempB> req_read_temp_b(create_command_crc(READ_TEMP_B));
            ReadRequest<uint16_t, StatusRegisterA> req_read_status_a(create_command_crc(READ_STATUS_A));
            ReadRequest<uint16_t, StatusRegisterB> req_read_status_b(create_command_crc(READ_STATUS_B));
        };

        namespace driver
        {
            bool discharging = false;
            SemaphoreHandle_t ltc6811_driver_semaphore;

            std::optional<bool> set_config(data::Config config)
            {
                registers::req_write_config_a.set(config);
                registers::req_write_config_a.request(SPI, SS);
                delayMicroseconds(500);
                auto res = registers::req_read_config_a.request(SPI, SS);
                if (!res.has_value())
                    return std::nullopt;
                registers::req_read_config_a.parse();
                return true;
            }

            std::optional<std::reference_wrapper<data::Config>> get_config()
            {
                auto res = registers::req_read_config_a.request(SPI, SS);
                if (!res.has_value())
                    return std::nullopt;
                registers::req_read_config_a.parse();
                return data::config;
            }

            std::optional<bool> set_duty(param::Duty duty)
            {
                data::Pwm pwm;
                for (auto &ic : pwm.data)
                {
                    for (auto &cell : ic)
                        cell = static_cast<uint8_t>(duty);
                }
                registers::req_write_pwm.set(pwm);
                registers::req_write_pwm.request(SPI, SS);
                delayMicroseconds(500);
                auto res = registers::req_read_pwm.request(SPI, SS);
                if (!res.has_value())
                    return std::nullopt;
                registers::req_read_pwm.parse();
                for (auto &ic : data::pwm.data)
                {
                    for (auto cell : ic)
                    {
                        if (cell != static_cast<uint8_t>(duty))
                        {
                            return std::nullopt;
                        }
                    }
                }
                return true;
            }

            std::optional<std::reference_wrapper<data::Pwm>> get_duty()
            {
                auto res = registers::req_read_pwm.request(SPI, SS);
                if (!res.has_value())
                    return std::nullopt;
                registers::req_read_pwm.parse();
                return data::pwm;
            }

            std::optional<std::reference_wrapper<data::Status>> get_status()
            {
                registers::req_start_conv_stat.request(SPI, SS);
                auto res_a = registers::req_read_status_a.request(SPI, SS);
                auto res_b = registers::req_read_status_b.request(SPI, SS);
                if (!res_a.has_value() || !res_b.has_value())
                    return std::nullopt;
                registers::req_read_status_a.parse();
                registers::req_read_status_b.parse();
                return data::status;
            }

            std::optional<std::reference_wrapper<data::CellVoltage>> get_cell()
            {
                data::cell_data.clear();
                registers::req_start_conv_cv.request(SPI, SS);
                auto res_a = registers::req_read_cell_a.request(SPI, SS);
                auto res_b = registers::req_read_cell_b.request(SPI, SS);
                auto res_c = registers::req_read_cell_c.request(SPI, SS);
                auto res_d = registers::req_read_cell_d.request(SPI, SS);
                if (!res_a.has_value() || !res_b.has_value() || !res_c.has_value() || !res_d.has_value())
                    return std::nullopt;
                registers::req_read_cell_a.parse();
                registers::req_read_cell_b.parse();
                registers::req_read_cell_c.parse();
                registers::req_read_cell_d.parse();
                data::cell_data.update_statistic();
                return data::cell_data;
            }

            std::optional<std::reference_wrapper<data::Temperature>> get_temp()
            {
                data::temp_data.clear();
                registers::req_start_conv_ax.request(SPI, SS);
                auto res_a = registers::req_read_temp_a.request(SPI, SS);
                auto res_b = registers::req_read_temp_b.request(SPI, SS);
                if (!res_a.has_value() || !res_b.has_value())
                    return std::nullopt;
                registers::req_read_temp_a.parse();
                registers::req_read_temp_b.parse();
                data::temp_data.update_statistic();
                return data::temp_data;
            }

            namespace discharge
            {
                enum class DeltaLine : uint8_t
                {
                    TOL_LINE,
                    ABS_LINE,
                };
                std::array<uint32_t, board::CHANE_LENGTH> voltage_delta;
                std::array<DeltaLine, board::CHANE_LENGTH> state{DeltaLine::TOL_LINE};
                SemaphoreHandle_t ltc6811_discharge_semaphore;
                data::Config config;
                uint32_t duty_count;

                void update_delta()
                {
                    for (size_t ic{}; ic < state.size(); ic++)
                    {
                        switch (state[ic])
                        {
                        case DeltaLine::TOL_LINE:
                            voltage_delta[ic] = param::VOL_DIF_TOL_LIMIT;
                            break;
                        case DeltaLine::ABS_LINE:
                            voltage_delta[ic] = param::VOL_DIF_ABS_LIMIT;
                            break;
                        }
                    }
                }

                void update_state(data::Config &data)
                {
                    for (size_t ic{}; ic < state.size(); ic++)
                    {
                        switch (state[ic])
                        {
                        case DeltaLine::TOL_LINE:
                            for (size_t i{}; i < data.data[ic].size(); i++)
                            {
                                if (data.data[ic][i])
                                {
                                    state[ic] = DeltaLine::ABS_LINE;
                                    discharging = true;
                                    break;
                                }
                            }
                            break;
                        case DeltaLine::ABS_LINE:
                            state[ic] = DeltaLine::TOL_LINE;
                            for (size_t i{}; i < data.data[ic].size(); i++)
                            {
                                if (data.data[ic][i])
                                {
                                    state[ic] = DeltaLine::ABS_LINE;
                                    discharging = true;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }

                struct Method_Min
                {
                    data::Config update_dcc(data::CellVoltage &voltage, data::Temperature &temperature)
                    {
                        data::Config config;
                        data::ic_id ic = board::CHANE_LENGTH - 1;
                        update_delta();
                        for (auto &board : config.data)
                        {
                            for (size_t cell{}; cell < board::CELL_NUM_PER_IC; cell++)
                                if (voltage.vol[ic][cell] > voltage.vol_range.first + voltage_delta[ic] && !temperature.over[ic])
                                    board[cell] = true;
                                else
                                    board[cell] = false;
                            ic--;
                        }
                        update_state(config);
                        return config;
                    }
                };

                struct Method_Mean
                {
                    data::Config update_dcc(data::CellVoltage &voltage, data::Temperature &temperature)
                    {
                        data::Config config;
                        data::ic_id ic = board::CHANE_LENGTH - 1;
                        update_delta();
                        for (auto &board : data::config.data)
                        {
                            for (size_t cell{}; cell < board::CELL_NUM_PER_IC; cell++)
                                if (voltage.vol[ic][cell] > voltage.average + voltage_delta[ic] && !temperature.over[ic])
                                    board[cell] = true;
                                else
                                    board[cell] = false;
                            ic--;
                        }
                        update_state(config);
                        return config;
                    }
                };

                template <class C>
                void update_config(data::CellVoltage &voltage, data::Temperature &temperature)
                {
                    C method;
                    config = method.update_dcc(voltage, temperature);
                }
            };

            void setup()
            {
                SPI.begin();
                SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
                pinMode(SS, OUTPUT);
                pinMode(MISO, INPUT);
                init = true;
                delay(20);
                utils::wakeup(SPI, SS);
                if (!set_duty(param::DUTY_RATIO).has_value())
                {
                    Serial.println("Error");
                }
                data::ltc6811_data_semaphore = xSemaphoreCreateBinary();
                ltc6811_driver_semaphore = xSemaphoreCreateBinary();
                xSemaphoreGive(data::ltc6811_data_semaphore);
                xSemaphoreGive(ltc6811_driver_semaphore);
            }

            void loop(uint32_t vol_recover_time)
            {
                xSemaphoreTake(ltc6811_driver_semaphore, portMAX_DELAY);
                data::Config clear{};
                set_config(clear);
                vTaskDelay(vol_recover_time); // Wait Voltage Recovery Time
                get_cell();
                get_temp();
                get_status();
                get_duty();
                get_config();
                discharge::update_config<discharge::Method_Min>(data::cell_data, data::temp_data);
                xSemaphoreGive(ltc6811_driver_semaphore);
            }
            void discharge_loop()
            {
                if (param::DISCHARGE_PERMISSION == param::Dcp::Enable)
                {
                    xSemaphoreTake(ltc6811_driver_semaphore, portMAX_DELAY);
                    if (static_cast<uint32_t>(param::DUTY_RATIO) >= discharge::duty_count)
                    {
                        set_config(discharge::config);
                        discharge::duty_count++;
                    }
                    xSemaphoreGive(ltc6811_driver_semaphore);
                    if (discharge::duty_count > param::Duty::Ratio_16_16)
                    {
                        discharge::duty_count = 0;
                    }
                }
            }
        };
    };

}
