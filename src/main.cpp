#include <Arduino.h>
// #include "ltc6811.h"
// #include "isl28022.h"
#include "bms_can.h"

// #include <string>

// LTC6811 bms(SPI);
// ISL28022 pm(Wire, 0b1000000);

// static constexpr bool DISCHARGE = true;

// void discharge_cell(std::optional<LTC6811VoltageStatus> &vol_status, std::optional<LTC6811TempStatus> &temp_status)
// {
//   if (!(vol_status.has_value() && temp_status.has_value()))
//     return;

//   bms.BuildDischargeConfig(vol_status.value(), temp_status.value());
//   delay(1000);
//   bms.ClearDischargeConfig();
//   delay(100); // <------電圧が回復するまで待ちたい
// }

// SPISettings mySPISettings = SPISettings(1000000, MSBFIRST, SPI_MODE0);
void setup()
{
  // SPI.begin();
  // SPI.beginTransaction(mySPISettings);
  // Wire.begin();
  Serial.begin(9600);
  // delay(20);
  // pinMode(SS, OUTPUT);
  // pinMode(MISO, INPUT);
  // Serial.println("cell 0, cell 1, cell 2, cell 3, cell 4, cell 5, cell 6, cell 7, cell 8, cell 9, cell 10, cell 11,");
  // bms.SetPwmDuty(LTC6811::Duty::Ratio_12_16);
  Serial.println("CAN_SETUP");
  can::driver::setup();
}

void loop()
{
  delay(1000);
  Serial.println("CAN_SEND");
  can::driver::report(100, 100, ltc6811::data::cell_data, ltc6811::data::temp_data);
  Serial.println("CAN_FINISH");
}

//   {
//     delay(1000);
//     Serial.println();

//     {
//       auto status = bms.GetGeneralStatus();
//       if (status.has_value())
//       {
//         // Serial.write("\r\nGeneralStatus:\r\n");
//         for (const auto board : status.value().data)
//         {
//           Serial.println(("LTCSumOfCell: " + std::to_string(static_cast<float>(board.SumOfCells)) + ",").c_str());
//           // Serial.println(("InternalDieTemp: " + std::to_string(static_cast<float>(board.InternalDieTemp)) + ",").c_str());
//           // Serial.println(("DigitalPower: " + std::to_string(static_cast<float>(board.Vdigital)) + ",").c_str());
//           // Serial.println(("AnalogPower: " + std::to_string(static_cast<float>(board.Vanalog)) + ",").c_str());
//         }
//       }
//     }
//     {
//       auto status = pm.GetBusVoltage();
//       if (status.has_value())
//       {
//         Serial.println(("ISLBUSVOLTAGE(V): " + std::to_string(status.value())).c_str());
//       }
//     }
//     {
//       auto status = pm.GetCurrent();
//       if (status.has_value())
//       {
//         Serial.println(("ISLCURRENT(A): " + std::to_string(status.value())).c_str());
//       }
//     }
//     {
//       auto status = pm.GetShuntVoltage();
//       if (status.has_value())
//       {
//         Serial.println(("ISLSHUNTVOLTAGE:(mV) " + std::to_string(status.value())).c_str());
//       }
//     }
//     {
//       auto status = pm.GetPower();
//       if (status.has_value())
//       {
//         Serial.println(("ISLPOWER(W): " + std::to_string(status.value())).c_str());
//       }
//     }
//   }
// }

// Serial.println();
// auto vol_status = bms.GetVoltageStatus();
// auto temp_status = bms.GetTemperatureStatus();

// if (vol_status.has_value() && temp_status.has_value())
// {
//   for (const auto &board : vol_status.value().vol)
//   {
//     for (const auto voltage : board)
//     {
//       Serial.write((std::to_string(static_cast<float>(voltage) / 10000) + ",").c_str());
//     }
//     Serial.println();
//   }
//   Serial.println();
//   Serial.write("\r\nCellVoltages:[V]\r\n");
//   Serial.write(("MAX: " + std::to_string(static_cast<float>(vol_status.value().max) / 10000) + "\r\n").c_str());
//   Serial.write(("MIN: " + std::to_string(static_cast<float>(vol_status.value().min) / 10000) + "\r\n").c_str());
//   Serial.write(("DIF: " + std::to_string(static_cast<float>(vol_status.value().max - vol_status.value().min) / 10000) + "\r\n").c_str());
//   Serial.write(("SUM: " + std::to_string(static_cast<float>(vol_status.value().sum) / 10000) + "\r\n").c_str());
//   Serial.println();
//   for (const auto &board : temp_status.value().temp)
//   {
//     for (const auto temp : board)
//     {
//       Serial.write((std::to_string(static_cast<float>(temp) / 1000) + ",").c_str());
//     }
//     Serial.println();
//   }
//   Serial.write("\r\nTemperatures:[deg]\r\n");
//   Serial.write(("MAX: " + std::to_string(static_cast<float>(temp_status.value().max) / 1000) + "\r\n").c_str());
//   Serial.write(("MIN: " + std::to_string(static_cast<float>(temp_status.value().min) / 1000) + "\r\n").c_str());
//   Serial.write("\r\n");
//   if (DISCHARGE)
//   {
//     discharge_cell(vol_status, temp_status);
//   }
//   else
//   {
//     delay(1000);
//   }
// }

// {
//   Serial.println();
//   delay(1000);
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
// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// #include <CAN.h>

// void setup()
// {
//   Serial.begin(9600);
//   while (!Serial)
//     ;

//   Serial.println("CAN Sender");

//   // start the CAN bus at 500 kbps
//   CAN.setPins(34, 32);
//   if (!CAN.begin(500E3))
//   {
//     Serial.println("Starting CAN failed!");
//     while (1)
//       ;
//   }
// }

// void loop()
// {
//   // send packet: id is 11 bits, packet can contain up to 8 bytes of data
//   Serial.print("Sending packet ... ");

//   CAN.beginPacket(0x12);
//   CAN.write('h');
//   CAN.write('e');
//   CAN.write('l');
//   CAN.write('l');
//   CAN.write('o');
//   CAN.endPacket();

//   Serial.println("done");

//   delay(1000);

//   // send extended packet: id is 29 bits, packet can contain up to 8 bytes of data
//   Serial.print("Sending extended packet ... ");

//   CAN.beginExtendedPacket(0xabcdef);
//   CAN.write('w');
//   CAN.write('o');
//   CAN.write('r');
//   CAN.write('l');
//   CAN.write('d');
//   CAN.endPacket();

//   Serial.println("done");

//   delay(1000);
// }