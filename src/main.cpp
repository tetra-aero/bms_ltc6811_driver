#include <Arduino.h>
#include "ltc6811.h"
#include <string>

LTC6811 bms(SPI);

void setup()
{
  pinMode(SS, OUTPUT);
  SPI.begin();
  Serial.begin(115200);
}

void loop()
{
  auto status = bms.GetVoltageStatus();
  if (status.has_value())
  {
    Serial.write(std::to_string(status.value().sum).c_str());
  }
}