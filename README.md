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