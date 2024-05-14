# BMS Firmware with LTC6811 and ISL28022

# 動作確認の取れている機能
- セル電圧測定
- サーミスタ温度測定
- セルバランシング

# 未動作確認
- シャント抵抗周り

# 放置

## プログラムのドキュメント

### ディージーチェーンの個数設定　
`ltc6811.h`の`constexpr static size_t kDaisyChainLength{1};`を変更
### セル電圧
`std::optional<LTC6811VoltageStatus> LTC6811::GetVoltageStatus(void)`
### サーミスタ温度
`std::optional<LTC6811TempStatus> LTC6811::GetTemperatureStatus(void)`
### SumofCells/DigitalPowerVoltage/AnalogPowerVoltage/InternalDieTemperature
`std::optional<LTC6811GeneralStatus> LTC6811::GetGeneralStatus()`
### 放電Duty比の設定
- 16段階でDutyの設定ができる(enum Dutyを参照 https://github.com/tetra-aero/bms_ltc6811_driver/blob/54e087a0274dd7d074bec60e3700e185a260857c/src/ltc6811.h#L155)
- LTC6811の機能としては，セルごとに設定できるがこのメソッドでは，すべてのセルに対して設定を適応する
- Duty比 0.75の例: https://github.com/tetra-aero/bms_ltc6811_driver/blob/54e087a0274dd7d074bec60e3700e185a260857c/src/main.cpp#L35C1-L35C46
`void LTC6811::SetPwmDuty(uint8_t ratio)`
