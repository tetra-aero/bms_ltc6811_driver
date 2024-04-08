#include <Arduino.h>
#include "ltc6811.h"
#include "isl28022.h"

#include <string>

LTC6811 bms(SPI);
ISL28022 pm(Wire, 0b1000000);

SPISettings mySPISettings = SPISettings(1000000, MSBFIRST, SPI_MODE0);
void setup()
{
  pinMode(SS, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(mySPISettings);
  // Wire.begin();
  Serial.begin(9600);
  delay(20);
  // pm.begin();
}

void loop()
{
  Serial.write("__LOOP__\r\n");
  {
    auto status = bms.GetVoltageStatus();
    Serial.write("\r\n");
    if (status.has_value())
    {
      Serial.write("VOLTAGE:[V]\r\n");
      for(auto board : status.value().vol){
        for(auto voltage : board){
          Serial.write((std::to_string(static_cast<float>(voltage) / 10000)+  "\r\n").c_str());
        }
      }
      Serial.write("\r\n");
    }
  }
   
  {
    auto status = bms.GetTemperatureStatus();
    if (status.has_value())
    {
      for(auto board : status.value().temp){
        for(auto temp : board){
          Serial.write((std::to_string(static_cast<float>(temp) / 1000)+  "\r\n").c_str());
        }
      }
      Serial.write("\r\nTemperature:[deg]\r\n");
      Serial.write(("MAX: "+std::to_string(static_cast<float>(status.value().max) / 1000) + "\r\n").c_str());
      Serial.write(("MIN: "+std::to_string(static_cast<float>(status.value().min) / 1000) + "\r\n").c_str());
      Serial.write("\r\n");
    }
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

  delay(1000);
}