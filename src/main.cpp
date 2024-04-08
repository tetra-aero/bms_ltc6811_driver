#include <Arduino.h>
#include "ltc6811.h"
#include "isl28022.h"

#include <string>

LTC6811 bms(SPI, LTC6811::Normal, LTC6811::Enabled, LTC6811::AllCell,LTC6811::AllAux,LTC6811::AllStat); // battery management ic
ISL28022 pm(Wire, 0b1000000); // power monitor ic
SPISettings mySPISettings = SPISettings(1000000, MSBFIRST, SPI_MODE0);

void BatteryModuleBalancingSetup()
{
  bms.SetDischargeMode(LTC6811::GTMinPlusDelta);
  return;
}

void BatteryModuleBalancingLoop()
{
  auto result = bms.GetVoltageStatus();
  if(result.has_value())
  {
    Serial.println(("MAX:" + std::to_string(result.value().max)).c_str());
    Serial.println(("MIN:" + std::to_string(result.value().min)).c_str());
    Serial.println(("DIF:" + std::to_string(result.value().max - result.value().min)).c_str());
    bms.BuildDischargeConfig(result.value());
    // bms.BuildDischargeConfig(result.value());
  }
  return;
}

void GeneralSetup()
{
  pinMode(SS, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(mySPISettings);
  // Wire.begin();
  Serial.begin(9600);
  delay(20);
  // pm.begin();
}

void DebugVolTempLoop()
{
  {
    auto status = bms.GetVoltageStatus();
    Serial.write("\r\n");
    if (status.has_value())
    {
      Serial.write("VOLTAGE:[mV]\r\n");
      for(auto board : status.value().vol){
        for(auto voltage : board){
          Serial.write((std::to_string(voltage / 10) + "\r\n").c_str());
        }
      }
      Serial.write("\r\n");
    }
  }
  {
    auto status = bms.GetTemperatureStatus();
    if (status.has_value())
    {
      Serial.write("\r\nTemperature:[mdeg]\r\n");
      Serial.write(("MAX: "+std::to_string(status.value().max) + "\r\n").c_str());
      Serial.write(("MIN: "+std::to_string(status.value().min) + "\r\n").c_str());
      Serial.write("\r\n");
    }
  }
}

void PowerManagementLoop()
{
  {
    auto status = pm.GetBusVoltage();
    if (status.has_value())
    {
      Serial.write("BUSVOLTAGE: ");
      Serial.write(std::to_string(status.value()).c_str());
      Serial.write("\r\n");
    }
  }
  {
    auto status = pm.GetCurrent();
    if (status.has_value())
    {
      Serial.write("CURRENT: ");
      Serial.write(std::to_string(status.value()).c_str());
      Serial.write("\r\n");
    }
  }
  {
    auto status = pm.GetShuntVoltage();
    if (status.has_value())
    {
      Serial.write("SHUNTVOLTAGE: ");
      Serial.write(std::to_string(status.value()).c_str());
      Serial.write("\r\n");
    }
  }
  {
    auto status = pm.GetPower();
    if (status.has_value())
    {
      Serial.write("POWER: ");
      Serial.write(std::to_string(status.value()).c_str());
      Serial.write("\r\n");
    }
  }
}


void setup()
{
  GeneralSetup();
  // BatteryModuleBalancingSetup();
}

void loop()
{
  Serial.write("__LOOP__\r\n");
  // BatteryModuleBalancingLoop();
  DebugVolTempLoop();

  // delay(500);
}