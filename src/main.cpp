#include <Arduino.h>
#include "ltc6811.h"
#include "isl28022.h"

#include <string>

LTC6811 bms(SPI);
ISL28022 pm(Wire, 0b1000000);

static constexpr bool DISCHARGE = true; 

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
  Serial.println("cell 0, cell 1, cell 2, cell 3, cell 4, cell 5, cell 6, cell 7, cell 8, cell 9, cell 10, cell 11,");
  bms.SetPwmDuty(LTC6811::Duty::Ratio_8_16);
}

void loop()
{
  {
    Serial.println();
    auto status = bms.GetVoltageStatus();

    if (status.has_value())
    {
      for (const auto board : status.value().vol)
      {
        for (const auto voltage : board)
        {
          Serial.write((std::to_string(static_cast<float>(voltage) / 10000) + ",").c_str());
        }
        Serial.println();
      }
      Serial.println();
      Serial.write("\r\nCellVoltages:[V]\r\n");
      Serial.write(("MAX: " + std::to_string(static_cast<float>(status.value().max) / 10000) + "\r\n").c_str());
      Serial.write(("MIN: " + std::to_string(static_cast<float>(status.value().min) / 10000) + "\r\n").c_str());
      Serial.write(("DIF: " + std::to_string(static_cast<float>(status.value().max - status.value().min) / 10000) + "\r\n").c_str());
      Serial.write(("SUM: " + std::to_string(static_cast<float>(status.value().sum) / 10000) + "\r\n").c_str());
      Serial.println();
      if(DISCHARGE){
        discharge_cell(std::move(status));
      }else{
        delay(1000);
      }
    }
  }

  // {
  //   Serial.println();
  //   // delay(1000);
  //   auto status = bms.GetGeneralStatus();
  //   if (status.has_value())
  //   {
  //     Serial.write("\r\nGeneralStatus:\r\n");
  //     for (const auto board : status.value().data)
  //     {
  //       Serial.write(("SumOfCell: " + std::to_string(static_cast<float>(board.SumOfCells)) + ",").c_str());
  //       Serial.write(("InternalDieTemp: " + std::to_string(static_cast<float>(board.InternalDieTemp)) + ",").c_str());
  //       Serial.write(("DigitalPower: " + std::to_string(static_cast<float>(board.Vdigital)) + ",").c_str());
  //       Serial.write(("AnalogPower: " + std::to_string(static_cast<float>(board.Vanalog)) + ",").c_str());
  //       Serial.println();
  //     }
      
  //   }
  // }

  // {
  //   Serial.println();
  //   auto status = bms.GetTemperatureStatus();
  //   if (status.has_value())
  //   {
  //     for (const auto board : status.value().temp)
  //     {
  //       for (const auto temp : board)
  //       {
  //         Serial.write((std::to_string(static_cast<float>(temp) / 1000) + ",").c_str());
  //       }
  //       Serial.println();
  //     }
  //     Serial.write("\r\nTemperatures:[deg]\r\n");
  //     Serial.write(("MAX: " + std::to_string(static_cast<float>(status.value().max) / 1000) + "\r\n").c_str());
  //     Serial.write(("MIN: " + std::to_string(static_cast<float>(status.value().min) / 1000) + "\r\n").c_str());
  //     Serial.write("\r\n");
  //   }
  // }
  // delay(1000);
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