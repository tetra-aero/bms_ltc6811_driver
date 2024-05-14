/*
 * LTC6811.cpp
 *
 *  Created on: 12 Mar 2020
 *      Author: Joshua
 *  Edited on : 22 Feb 2024
 *      Author: Hori
 */

#include "ltc6811.h"

LTC6811::LTC6811(SPIClass &hspi, Mode mode, DCP dcp, CellCh cell, AuxCh aux, STSCh sts)
    : hspi{hspi}
{

    uint8_t md_bits = (mode & 0x02) >> 1;
    uint16_t PEC{0};

    ADCV[0] = 0x02 | md_bits;
    ADAX[0] = 0x04 | md_bits;
    ADSTAT[0] = 0x04 | md_bits;

    md_bits = (mode & 0x01) << 7;
    ADCV[1] = md_bits | 0x60 | dcp << 4 | cell;
    ADAX[1] = md_bits | 0x60 | aux;
    ADSTAT[1] = md_bits | 0x68 | sts;

    PEC = PEC15Calc(ADCV, 2);
    ADCV[2] = static_cast<uint8_t>(PEC >> 8);
    ADCV[3] = static_cast<uint8_t>(PEC);

    PEC = PEC15Calc(ADAX, 2);
    ADAX[2] = static_cast<uint8_t>(PEC >> 8);
    ADAX[3] = static_cast<uint8_t>(PEC);

    PEC = PEC15Calc(ADSTAT, 2);
    ADSTAT[2] = static_cast<uint8_t>(PEC >> 8);
    ADSTAT[3] = static_cast<uint8_t>(PEC);

    slave_cfg_tx.register_group.fill({0xFC, 0, 0, 0, 0, 0});

    WakeFromSleep(); // TODO Takes 2.2s to fall asleep so if this has to be called after this, we have problems
}

void LTC6811::WakeFromSleep(void)
{
    for (size_t i = 0; i < kDaisyChainLength; ++i)
    {
        digitalWrite(SS, LOW);
        delayMicroseconds(T_WAKE_MAX); // Guarantees the LTC6811 will be in standby
        digitalWrite(SS, HIGH);
        delayMicroseconds(10);
    }
}

void LTC6811::WakeFromIdle(void)
{
    uint8_t const data = 0xFF;

    for (size_t i = 0; i < kDaisyChainLength; ++i)
    {
        digitalWrite(SS, LOW);
        hspi.transfer(data);
        digitalWrite(SS, HIGH);
    }
}

bool LTC6811::ReadPWMRegisterGroup(void)
{
    return ReadRegisterGroup(slave_pwm_rx);
}

bool LTC6811::WritePWMRegisterGroup(void)
{
    return WriteRegisterGroup(slave_pwm_tx);
}

/* Read a cell voltage register group of an LTC6811 daisy chain.
 * Returns 0 on success, 1 if either PEC or SPI error.
 */
bool LTC6811::ReadVoltageRegisterGroup(Group const group)
{
    return ReadRegisterGroup(cell_data[group]);
}

/* Read an auxiliary register group of an LTC6811 daisy chain.
 * Returns 0 on success, 1 if either PEC or SPI error.
 */
bool LTC6811::ReadAuxRegisterGroup(Group const group)
{
    return ReadRegisterGroup(temp_data[group]);
}

/* Read a status register group of an LTC6811 daisy chain. */
bool LTC6811::ReadStatusRegisterGroup(Group const group)
{
    return ReadRegisterGroup(status_registers[group]);
}

/* Read the configuration register group of an LTC6811 daisy chain */
bool LTC6811::ReadConfigRegisterGroup(void)
{
    return ReadRegisterGroup(slave_cfg_rx);
}

/* Write to the configuration register group of an LTC6811 daisy chain. */
bool LTC6811::WriteConfigRegisterGroup(void)
{
    return WriteRegisterGroup(slave_cfg_tx);
}

/* Clear the LTC6811 cell voltage registers. */
void LTC6811::ClearVoltageRegisters(void)
{
    constexpr static LTC6811Command command{7, 17, 201, 192};
    WakeFromIdle();
    digitalWrite(SS, LOW);
    hspi.writeBytes(command.data(), kCommandLength);
    digitalWrite(SS, HIGH);
}

/* Clear the LTC6811 Auxiliary registers. */
void LTC6811::ClearAuxRegisters(void)
{
    constexpr static LTC6811Command command{7, 18, 223, 164};
    WakeFromIdle();
    digitalWrite(SS, LOW);
    hspi.writeBytes(command.data(), kCommandLength);
    digitalWrite(SS, HIGH);
}

std::optional<LTC6811PWMRegisterStatus> LTC6811::GetPwmStatus()
{
    LTC6811PWMRegisterStatus status{};

    if (!ReadPWMRegisterGroup())
    {
        return std::nullopt;
    }
    size_t board_id{};
    size_t cell_id{};
    for (const auto &register_group : slave_pwm_rx.register_group)
    {
        for (const auto &data : register_group.data)
        {
            status.pwm[board_id][cell_id++] = data & 0x0F;
            status.pwm[board_id][cell_id++] = (data >> 4) & 0x0F;
        }
        cell_id = 0;
        board_id++;
    }

    return status;
}

std::optional<LTC6811GeneralStatus> LTC6811::GetGeneralStatus()
{
    LTC6811GeneralStatus status{};

    StartConversion(ADSTAT);

    for (size_t group = A; group <= B; ++group)
        if (!ReadStatusRegisterGroup(static_cast<Group>(group)))
            return std::nullopt;

    size_t board_id{};
    size_t register_count{};
    for (const auto &register_group : status_registers)
    {
        board_id = 0;
        for (const auto &Register : register_group.register_group)
        {

            int register_number = (register_count >= kDaisyChainLength) ? Group::B : Group::A;
            if (register_number == Group::A)
            {
                uint16_t sc = Register.data[0];
                uint16_t itmp = Register.data[1];
                uint16_t va = Register.data[2];
                status.data[board_id].SumOfCells = static_cast<float>(sc) / 10000.0 * 20.0;
                status.data[board_id].InternalDieTemp = (static_cast<float>(itmp) / 10.0 / 7.5) - 273.15;
                status.data[board_id].Vanalog = static_cast<float>(va) / 10000.0;
            }
            else if (register_number == Group::B)
            {
                uint16_t vd = Register.data[0];
                status.data[board_id].Vdigital = static_cast<float>(vd) / 10000.0;
            }
            register_count++;
            board_id++;
        }
    }

    return status;
}

/* Generate a status report of the cell voltage register groups.
 * Returns an LTC6811VoltageStatus on success, nullopt if error
 */
std::optional<LTC6811VoltageStatus> LTC6811::GetVoltageStatus(void)
{
    LTC6811VoltageStatus status{};
    size_t count{0};
    std::array<uint32_t, kDaisyChainLength> index_count{0};

    StartConversion(ADCV);

    for (size_t group = A; group <= D; ++group)
        if (!ReadVoltageRegisterGroup(static_cast<Group>(group)))
            return std::nullopt;

    for (const auto &register_group : cell_data)
    {
        for (const auto &Register : register_group.register_group)
        {
            auto board_id = (count / 3) % kDaisyChainLength;
            for (const auto voltage : Register.data)
            {
                status.vol[board_id][index_count[board_id]] = voltage;

                status.sum += voltage;

                if (voltage < status.min)
                {
                    status.min = voltage;
                    status.min_id = {board_id, index_count[board_id]};
                }
                else if (voltage > status.max)
                {
                    status.max = voltage;
                    status.max_id = {board_id, index_count[board_id]};
                }
                index_count[board_id]++;
                ++count;
            }
        }
    }
    return status;
}

/* Generate a status report of the current temperatures from aux voltage register groups.
 * Returns an LTC6811TempStatus on success, nullopt if error
 */
std::optional<LTC6811TempStatus> LTC6811::GetTemperatureStatus()
{
    LTC6811TempStatus status{};
    size_t count{0};
    std::array<uint32_t, kDaisyChainLength> index_count{0};

    auto Bvalue = [](int16_t const NTC_voltage) noexcept
    {
        constexpr auto Vin = 30000.0;
        constexpr auto B = 4550;
        auto R = NTC_voltage / (Vin - NTC_voltage);
        auto TempInv = ((1 / (273.15 + 25)) + (std::log(R) / B));
        return static_cast<int32_t>(((1 / TempInv) * 1000 - 273150));
    };

    for (int i = 0; i < 3; i++)
    {
        StartConversion(ADAX);
    }

    for (size_t group = A; group <= B; ++group)
        if (!ReadAuxRegisterGroup(static_cast<Group>(group)))
            return std::nullopt;

    for (const auto &register_group : temp_data)
    {
        for (const auto &Register : register_group.register_group)
        {
            auto board_id = (count / 3) % kDaisyChainLength;
            for (auto data : Register.data)
            {
                if (index_count[board_id] == 5)
                {
                    status.vref2[board_id] = data;
                }
                else
                {
                    int32_t temperature = Bvalue(data);
                    status.temp[board_id][index_count[board_id]++] = temperature;

                    if (temperature < status.min)
                    {
                        status.min = temperature;
                        status.min_id = count;
                    }
                    if (temperature > status.max)
                    {
                        status.max = temperature;
                        status.max_id = count;
                    }
                }
                ++count;
            }
        }
    }
    return status;
}

void LTC6811::BuildDischargeConfig(const LTC6811VoltageStatus &voltage_status, const LTC6811TempStatus &temp_status)
{
    uint16_t DCCx{0};
    uint8_t current_cell{0}, current_ic{kDaisyChainLength - 1};
    uint8_t kDelta{0};

    switch (barancing_state) //それぞれ状態のkDeltaの設定
    {
    case OverAbsoleteLine:
        kDelta = kDeltaAbsolete;
        break;
    case Complete:
        kDelta = kDeltaTolerant;
        break;
    default:
        break;
    }

    switch (discharge_mode)
    {
    case GTMinPlusDelta:
        for (auto &cfg_register : slave_cfg_tx.register_group) // 　前から n-1, n-2, ..., 0
        {
            DCCx = 0;
            bool overTemp = false;

            for (auto temp : temp_status.temp[current_ic])
            {
                if (temp > tolerantTemp)
                    overTemp = true;
            }

            if (overTemp)
            {
                Serial.println("OverTemp");
                continue;
            }

            for (int cell{}; cell < 12; cell++)
            {
                if (voltage_status.vol[current_ic][cell] > voltage_status.min + kDelta)
                    DCCx |= (1 << cell);
            }

            if (DCCx == 0) //どのセルも放電していない時
            {
                if (barancing_state == DisChargeState::OverAbsoleteLine) //状態が放電状態なら許容ラインまで判定条件を緩和する
                    barancing_state = DisChargeState::Complete;
            }
            else  // どれか放電している時
            {
                if (barancing_state == DisChargeState::Complete)
                {
                    barancing_state = DisChargeState::OverAbsoleteLine; //状態が完了状態ならkDeltaを絶対ラインまで引き下げる
                }
            }

            Serial.println(("Discarge : " + std::to_string(current_ic) + "-" + std::to_string(DCCx)).c_str());
            current_ic--;
            cfg_register.data[4] |= DCCx & 0xFF;
            cfg_register.data[5] |= DCCx >> 8 & 0xF;
            cfg_register.PEC = PEC15Calc(cfg_register.data);
        }
        break;

    case MaxOnly:
        if (voltage_status.max - voltage_status.min > kDelta)
        {
            current_ic = kDaisyChainLength - 1 - voltage_status.max_id.first;

            bool overTemp = false;

            for (auto temp : temp_status.temp[current_ic])
            {
                if (temp > tolerantTemp)
                    overTemp = true;
            }

            if (overTemp)
            {
                Serial.println("OverTemp");
                break;
            }

            Serial.println(("Discarge : " + std::to_string(voltage_status.max_id.first) + "-" + std::to_string(voltage_status.max_id.second)).c_str());
            DCCx |= 1 << voltage_status.max_id.second;

             if (DCCx == 0) //どのセルも放電していない時
            {
                if (barancing_state == DisChargeState::OverAbsoleteLine) //状態が放電状態なら許容ラインまで判定条件を緩和する
                    barancing_state = DisChargeState::Complete;
            }
            else  // どれか放電している時
            {
                if (barancing_state == DisChargeState::Complete)
                {
                    barancing_state = DisChargeState::OverAbsoleteLine; //状態が完了状態ならkDeltaを絶対ラインまで引き下げる
                }
            }

            slave_cfg_tx.register_group[current_ic].data[4] = DCCx & 0xFF;
            slave_cfg_tx.register_group[current_ic].data[5] = DCCx >> 8 & 0xF;
            slave_cfg_tx.register_group[current_ic].PEC = PEC15Calc(slave_cfg_tx.register_group[current_ic].data);
        }
        break;

    case GTMeanPlusDelta:
        size_t average_voltage{voltage_status.sum / (12 * kDaisyChainLength)};

        for (auto &cfg_register : slave_cfg_tx.register_group)
        {
            DCCx = 0;
            current_cell = 0;
            bool overTemp = false;

            for (auto temp : temp_status.temp[current_ic])
            {
                if (temp > tolerantTemp)
                    overTemp = true;
            }

            if (overTemp)
            {
                Serial.println("OverTemp");
                continue;
            }
            for (int cell{}; cell < 12; cell++)
            {
                if (voltage_status.vol[current_ic][cell] > average_voltage + kDelta)
                    DCCx |= (1 << cell);
            }

            if (DCCx == 0) //どのセルも放電していない時
            {
                if (barancing_state == DisChargeState::OverAbsoleteLine) //状態が放電状態なら許容ラインまで判定条件を緩和する
                    barancing_state = DisChargeState::Complete;
            }
            else  // どれか放電している時
            {
                if (barancing_state == DisChargeState::Complete)
                {
                    barancing_state = DisChargeState::OverAbsoleteLine; //状態が完了状態ならkDeltaを絶対ラインまで引き下げる
                }
            }

            Serial.println(("Discarge : " + std::to_string(current_ic) + "-" + std::to_string(DCCx)).c_str());
            current_ic--;
            cfg_register.data[4] |= DCCx & 0xFF;
            cfg_register.data[5] |= DCCx >> 8 & 0xF;
            cfg_register.PEC = PEC15Calc(cfg_register.data);
        }
        break;
    }
    WriteConfigRegisterGroup();
    delayMicroseconds(500); // TODO take this out. Just read when we need the data to send over CAN or whatever
    ReadConfigRegisterGroup();
    // Serial.println("Discharge");
    // for (auto x : slave_cfg_rx.register_group)
    // {
    //     for (auto y : x.data)
    //     {
    //         Serial.write((std::to_string(y) + " ").c_str());
    //     }

    // }
    // Serial.println();
}

void LTC6811::SetPwmDuty(uint8_t ratio)
{
    for (auto &register_group : slave_pwm_tx.register_group)
    {
        uint8_t pwm_reg_val = static_cast<uint8_t>((ratio) << 4 | ratio);
        register_group.data = {pwm_reg_val, pwm_reg_val, pwm_reg_val, pwm_reg_val, pwm_reg_val, pwm_reg_val};
        register_group.PEC = PEC15Calc(register_group.data);
    }
    WritePWMRegisterGroup();
    delayMicroseconds(500);
    ReadPWMRegisterGroup();

    Serial.println();
    for (auto x : slave_pwm_rx.register_group)
    {
        for (auto y : x.data)
        {
            Serial.write((std::to_string(y) + " ").c_str());
        }
    }
}

void LTC6811::ClearDischargeConfig()
{
    uint16_t DCCx = 0;
    for (auto &register_group : slave_cfg_tx.register_group)
    {
        register_group.data[4] = DCCx & 0xFF;
        register_group.data[5] = DCCx >> 8 & 0xFF;
        register_group.PEC = PEC15Calc(register_group.data);
    }
    WriteConfigRegisterGroup();
    delayMicroseconds(500); // TODO take this out. Just read when we need the data to send over CAN or whatever
    ReadConfigRegisterGroup();
    // Serial.println("Clear");
    // for (auto x : slave_cfg_rx.register_group)
    // {
    //     for (auto y : x.data)
    //     {
    //         Serial.write((std::to_string(y) + " ").c_str());
    //     }
    // }
}

/* Start a conversion */
void LTC6811::StartConversion(const LTC6811Command &command)
{
    WakeFromIdle(); // It's possible all of these can be removed

    digitalWrite(SS, LOW);
    delayMicroseconds(500);
    hspi.writeBytes(command.data(), kCommandLength); // Start cell voltage conversion.
    delayMicroseconds(500);
    digitalWrite(SS, HIGH);

    // while(!digitalRead(MOSI)){
    //     delayMicroseconds(100);
    // }

    delayMicroseconds((T_REFUP_MAX + T_CYCLE_FAST_MAX)); // TODO we aren't in fast conversion mode??? Also these delays aren't in the Linduino library
}