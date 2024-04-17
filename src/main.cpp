#include <Arduino.h>
#include "ltc6811.h"
#include "isl28022.h"

#include <string>

LTC6811 bms(SPI);
ISL28022 pm(Wire, 0b1000000);

void discharge_cell(std::optional<LTC6811VoltageStatus> &&status)
{
  bms.BuildDischargeConfig(status.value());
  delay(1000);
  bms.ClearDischargeConfig();
  delay(1);
}

SPISettings mySPISettings = SPISettings(1000000, MSBFIRST, SPI_MODE0);
void setup()
{

  SPI.begin();
  SPI.beginTransaction(mySPISettings);
  // Wire.begin();
  Serial.begin(9600);
  delay(20);
  // pm.begin();
  pinMode(SS, OUTPUT);
  // pinMode(MISO,INPUT);
  Serial.println("cell 0, cell 1, cell 2, cell 3, cell 4, cell 5, cell 6, cell 7, cell 8, cell 9, cell 10, cell 11, max - min");
  bms.StartConversion(bms.ADCV);
  bms.StartConversion(bms.ADAX);
  bms.StartConversion(bms.ADSTAT);
  bms.SetPwmDuty();
}

void loop()
{
  {
    auto status = bms.GetVoltageStatus();
    Serial.write("\r\n");
    if (status.has_value())
    {
      int count_board{};
      int count_cell{};
      for (const auto board : status.value().vol)
      {
        for (const auto voltage : board)
        {
          Serial.write((std::to_string(static_cast<float>(voltage) / 10000) + ",").c_str());
        }

        count_cell = 0;
        count_board++;
      }
      Serial.write((std::to_string(static_cast<float>(status.value().max - status.value().min) / 10000) + ",").c_str());
      Serial.write((std::to_string(static_cast<float>(status.value().sum) / 10000) + ",").c_str());
      discharge_cell(std::move(status));
      // delay(1000);
    }
  }

  // {
  //   auto status = bms.GetTemperatureStatus();
  //   if (status.has_value())
  //   {
  //     for (const auto board : status.value().temp)
  //     {
  //       for (const auto temp : board)
  //       {
  //         Serial.write((std::to_string(static_cast<float>(temp) / 1000) + "\r\n").c_str());
  //       }
  //     }
  //     Serial.write("\r\nTemperature:[deg]\r\n");
  //     Serial.write(("MAX: " + std::to_string(static_cast<float>(status.value().max) / 1000) + "\r\n").c_str());
  //     Serial.write(("MIN: " + std::to_string(static_cast<float>(status.value().min) / 1000) + "\r\n").c_str());
  //     Serial.write("\r\n");
  //   }
  // }
}

// {
//   auto status = pm.GetBusVoltage();
//   if (status.has_value())
//   {
//     Serial.write("BUSVOLTAGE: ");
//     Serial.write(std::to_string(status.value()).c_str());
//     Serial.write("\r\n");
//   }
// }
// {
//   auto status = pm.GetCurrent();
//   if (status.has_value())
//   {
//     Serial.write("CURRENT: ");
//     Serial.write(std::to_string(status.value()).c_str());
//     Serial.write("\r\n");
//   }
// }
// {
//   auto status = pm.GetShuntVoltage();
//   if (status.has_value())
//   {
//     Serial.write("SHUNTVOLTAGE: ");
//     Serial.write(std::to_string(status.value()).c_str());
//     Serial.write("\r\n");
//   }
// }
// {
//   auto status = pm.GetPower();
//   if (status.has_value())
//   {
//     Serial.write("POWER: ");
//     Serial.write(std::to_string(status.value()).c_str());
//     Serial.write("\r\n");
//   }
// }