# BMS Firmware with LTC6811 and ISL28022

# 動作確認の取れている機能
- セル電圧測定
- サーミスタ温度測定
- セルバランシング

# 未動作確認
- シャント抵抗周り

# 放置

## プログラムのドキュメント
## LTC6811
### ディージーチェーンの個数設定　
`ltc6811.h`の`constexpr static size_t kDaisyChainLength{1};`を変更
### セル電圧
`std::optional<LTC6811VoltageStatus> LTC6811::GetVoltageStatus(void)`
### サーミスタ温度
`std::optional<LTC6811TempStatus> LTC6811::GetTemperatureStatus(void)`
### SumofCells/DigitalPowerVoltage/AnalogPowerVoltage/InternalDieTemperature
`std::optional<LTC6811GeneralStatus> LTC6811::GetGeneralStatus()`
### 放電Duty比の設定
`void LTC6811::SetPwmDuty(uint8_t ratio)`
- 16段階でDutyの設定ができる(enum Dutyを参照 https://github.com/tetra-aero/bms_ltc6811_driver/blob/54e087a0274dd7d074bec60e3700e185a260857c/src/ltc6811.h#L155)
- LTC6811の機能としては，セルごとに設定できるがこのメソッドでは，すべてのセルに対して設定を適応する
- Duty比 0.75の例: https://github.com/tetra-aero/bms_ltc6811_driver/blob/54e087a0274dd7d074bec60e3700e185a260857c/src/main.cpp#L35
### 各種パラメータの設定
- ディージーチェーンの長さの設定 `kDaisyChainLength` https://github.com/tetra-aero/bms_ltc6811_driver/blob/f07c24927c33e81a0cd155b7a73bf68cf2515267/src/ltc6811.h#L29
- 放電時に許容するPCB温度(45度の時,45000) `tolerantTemp` https://github.com/tetra-aero/bms_ltc6811_driver/blob/f07c24927c33e81a0cd155b7a73bf68cf2515267/src/ltc6811.h#L32
- 放電時に許容するセル間電圧差(3mvの時,30) `kDelta` https://github.com/tetra-aero/bms_ltc6811_driver/blob/f07c24927c33e81a0cd155b7a73bf68cf2515267/src/ltc6811.h#L31

## ISL28022
- バス電圧(Master基板の)
https://github.com/tetra-aero/bms_ltc6811_driver/blob/b0f95185e32b73cf9b2c4d1f3ab613a96bfe6ee7/src/isl28022.h#L156
`std::optional<float> GetBusVoltage()`
- 電流
https://github.com/tetra-aero/bms_ltc6811_driver/blob/b0f95185e32b73cf9b2c4d1f3ab613a96bfe6ee7/src/isl28022.h#L182
`std::optional<float> GetCurrent()`
- シャント抵抗の電圧差
https://github.com/tetra-aero/bms_ltc6811_driver/blob/b0f95185e32b73cf9b2c4d1f3ab613a96bfe6ee7/src/isl28022.h#L143
`std::optional<float> GetShuntVoltage()`
- 電力
https://github.com/tetra-aero/bms_ltc6811_driver/blob/b0f95185e32b73cf9b2c4d1f3ab613a96bfe6ee7/src/isl28022.h#L169
`std::optional<float> GetPower()`
- それぞれの値の定数値の計算
https://github.com/tetra-aero/bms_ltc6811_driver/blob/b0f95185e32b73cf9b2c4d1f3ab613a96bfe6ee7/src/isl28022.h#L102C5-L124C6
