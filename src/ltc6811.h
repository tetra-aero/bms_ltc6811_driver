/*
 * LTC6820.h
 *
 *  Created on: 12 Mar 2020
 *      Author: Joshua
 *  Edited on : 22 Feb 2024
 *      Author: Hori
 */

#pragma once

#ifndef LTC6811_H_
#define LTC6811_H_
#include "Arduino.h"
#include "SPI.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <optional>

/* Timing of states (in microseconds) */
#define T_WAKE_MAX 400
#define T_READY 10
#define T_IDLE_MIN 4300
#define T_REFUP_MAX 4400
#define T_CYCLE_FAST_MAX 1185 // Measure 12 Cells

constexpr static size_t kBytesPerRegister{8};
constexpr static size_t kDaisyChainLength{1};
constexpr static size_t kCommandLength{4};
constexpr static uint32_t kDelta{100};
constexpr static uint32_t tolerantTemp{80000}; // 80度 

using LTC6811Command = std::array<uint8_t, kCommandLength>;

template <typename T>
struct LTC6811Register
{
    /* An LTC6811 register is 8 bytes: 6 bytes of data, and 2 bytes of PEC. sizeof(LTC6811<T>) will always return 8 */
    std::array<T, 6 / sizeof(T)> data;
    uint16_t PEC;
};

template <typename T>
struct LTC6811RegisterGroup
{
    /* This class bundles together the command to access some register group and data sent/received after that command */
    LTC6811Command const command;
    std::array<LTC6811Register<T>, kDaisyChainLength> register_group;
    LTC6811RegisterGroup(LTC6811Command &&command) : command{std::move(command)} {};
};

struct LTC6811VoltageStatus
{
    using board_id = uint8_t;
    using cell_id = uint8_t;
    int sum{0};
    std::array<std::array<uint16_t, 12>, kDaisyChainLength> vol;
    uint16_t min{std::numeric_limits<uint16_t>::max()};
    std::pair<board_id, cell_id> min_id{0xFF, 0xFF};
    uint16_t max{std::numeric_limits<uint16_t>::min()};
    std::pair<board_id, cell_id> max_id{0xFF, 0xFF};
};

struct LTC6811TempStatus
{
    std::array<std::array<int32_t, 5>, kDaisyChainLength> temp;
    std::array<int32_t, kDaisyChainLength> vref2;
    int32_t min{std::numeric_limits<int32_t>::max()};
    size_t min_id{0};
    int32_t max{std::numeric_limits<int32_t>::min()};
    size_t max_id{0};
};

struct LTC6811GeneralStatus
{
    struct Data
    {
        float SumOfCells;
        float InternalDieTemp;
        float Vdigital;
        float Vanalog;
    };
    std::array<Data,kDaisyChainLength> data;
};

struct LTC6811PWMRegisterStatus
{
    std::array<std::array<uint8_t, 12>, kDaisyChainLength> pwm;
};

class LTC6811
{
public:
    /* Conversion mode */
    enum Mode
    {
        Fast = 1,
        Normal,
        Filtered
    };
    /* Conversion channels */
    enum CellCh
    {
        AllCell,
        OneAndSeven,
        TwoAndEight,
        ThreeAndNine,
        FourAndTen,
        FiveAndEleven,
        SixAndTwelve
    };
    /* Conversion channels */
    enum AuxCh
    {
        AllAux,
        GPIO1,
        GPIO2,
        GPIO3,
        GPIO4,
        GPIO5,
        VREF2
    };
    /* Conversion channels */
    enum STSCh
    {
        AllStat,
        SOC,
        ITMP,
        VA,
        VD
    };
    /* Controls if Discharging transistors are enabled or disabled during Cell conversions. */
    enum DCP
    {
        Disabled,
        Enabled
    };

    enum Group
    {
        A,
        B,
        C,
        D
    };

    enum DischargeMode
    {
        GTMinPlusDelta,
        MaxOnly,
        GTMeanPlusDelta
    };

    enum Duty
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

    LTC6811(SPIClass &hspi, Mode mode = Mode::Normal, DCP dcp = DCP::Enabled,
            CellCh cell = AllCell, AuxCh aux = AllAux, STSCh sts = AllStat);

    void WakeFromSleep(void);

    void WakeFromIdle(void);

    bool WritePWMRegisterGroup(void);

    bool ReadPWMRegisterGroup(void);

    /* Read from an LTC6811 cell voltage register group. */
    bool ReadVoltageRegisterGroup(Group const group);

    /* Read from an LTC6811 auxiliary register group. */
    bool ReadAuxRegisterGroup(Group const group);

    /* Read from an LTC6811 status register group. */
    bool ReadStatusRegisterGroup(Group const group);

    /* Read from an LTC6811 configuration register group */
    bool ReadConfigRegisterGroup(void);

    /* Write to the configuration registers of the LTC6811s in the daisy chain. */
    bool WriteConfigRegisterGroup(void);

    /* Clear the LTC6811 cell voltage registers. */
    void ClearVoltageRegisters(void);

    /* Clear the LTC6811 Auxiliary registers. */
    void ClearAuxRegisters(void);

    void ClearDischargeConfig(void);

    [[nodiscard]] std::optional<LTC6811PWMRegisterStatus> GetPwmStatus(void);

    [[nodiscard]] std::optional<LTC6811GeneralStatus> GetGeneralStatus(void);

    [[nodiscard]] std::optional<LTC6811VoltageStatus> GetVoltageStatus(void);

    [[nodiscard]] std::optional<LTC6811TempStatus> GetTemperatureStatus(void);

    void BuildDischargeConfig(const LTC6811VoltageStatus &voltage_status,  const LTC6811TempStatus &temp_status);

    void SetPwmDuty(uint8_t ratio);

    void SetDischargeMode(DischargeMode const discharge_mode) noexcept
    {
        this->discharge_mode = discharge_mode;
    };

    [[nodiscard]] const auto &GetCellData() const noexcept { return cell_data; };

    LTC6811Command ADCV;   // Cell Voltage conversion command
    LTC6811Command ADAX;   // Aux conversion command
    LTC6811Command ADSTAT; // Status conversion command

    /* Start a Cell Voltage, Aux, Status, etc. Conversion */
    void StartConversion(const LTC6811Command &command);

private:
    SPIClass &hspi;

    DischargeMode discharge_mode{GTMeanPlusDelta};

    LTC6811RegisterGroup<uint8_t> slave_cfg_tx{LTC6811Command{0x00, 0x01, 0x3D, 0x6E}};
    LTC6811RegisterGroup<uint8_t> slave_cfg_rx{LTC6811Command{0x00, 0x02, 0x2B, 0x0A}};
    LTC6811RegisterGroup<uint8_t> slave_pwm_tx{LTC6811Command{0x00, 0x20, 0x00, 0x00}};
    LTC6811RegisterGroup<uint8_t> slave_pwm_rx{LTC6811Command{0x00, 0x22, 0x9D, 0x56}};
    std::array<LTC6811RegisterGroup<uint16_t>, 4> cell_data{
        LTC6811Command{0, 4, 7, 194}, LTC6811Command{0, 6, 154, 148}, LTC6811Command{0, 8, 94, 82}, LTC6811Command{0, 10, 195, 4}};
    std::array<LTC6811RegisterGroup<int16_t>, 2> temp_data{
        LTC6811Command{0, 12, 239, 204}, LTC6811Command{0, 14, 114, 154}};
    std::array<LTC6811RegisterGroup<uint16_t>, 2> status_registers{
        LTC6811Command{0x00, 0x10, 0xED, 0x72}, LTC6811Command{0x00, 0x12, 0x70, 0x24}};

    /* Write Register Function. Return 0 if success, 1 if failure. */
    template <typename T>
    bool WriteRegisterGroup(LTC6811RegisterGroup<T> &register_group)
    {
        WakeFromIdle();
        digitalWrite(SS, LOW);
        hspi.writeBytes(register_group.command.data(), sizeof(register_group.command));
        std::array<uint8_t, 8 * kDaisyChainLength> packet{0};
        int index{};
        for (auto x : register_group.register_group)
        {
            for (auto y : x.data)
            {
                packet[index++] = y;
            }
            packet[index++] = (x.PEC >> 8) & 0xFF;
            packet[index++] = x.PEC & 0xFF;
        }
        hspi.writeBytes(packet.data(), packet.size());
        digitalWrite(SS, HIGH);
        return false;
    }

    /* Read Register Function. Return 0 if success, 1 if failure. */
    template <typename T>
    bool ReadRegisterGroup(LTC6811RegisterGroup<T> &register_group)
    {
        WakeFromIdle();

        auto serialized_data = reinterpret_cast<uint8_t *>(register_group.register_group.begin());

        digitalWrite(SS, LOW);

        hspi.writeBytes(register_group.command.data(), kCommandLength);

        digitalWrite(SS, LOW);

        for (size_t i = 0; i < kBytesPerRegister * kDaisyChainLength; ++i)
        {
            serialized_data[i] = hspi.transfer(0xFF);
        }

        digitalWrite(SS, HIGH);

        for (auto &Register : register_group.register_group)
        {
            if (Register.PEC != PEC15Calc(Register.data))
            {
                return 1; // PEC error
            }
        }
        return false;
    }

    constexpr static uint16_t crc15Table[256]{
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

    /* This has been tested against the original code and is working properly */
public:
    template <typename T, size_t S>
    constexpr static uint16_t PEC15Calc(const std::array<T, S> &data, size_t size = S * sizeof(T))
    {
        uint16_t PEC{16}, addr{0};
        auto serialized_data = reinterpret_cast<uint8_t const *>(data.data());

        for (uint8_t i = 0; i < size; ++i)
        {
            addr = (PEC >> 7 ^ serialized_data[i]) & 0xFF;
            PEC = PEC << 8 ^ crc15Table[addr];
        }

        return PEC << 1; // From documentation: The final PEC is the 15-bit value in the PEC register with a 0 bit appended to its LSB.
    }
};
#endif /* LTC6811_H_ */