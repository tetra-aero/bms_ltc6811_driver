#include <Arduino.h>
#include "ltc6811.h"
#include <string>

LTC6811 bms(SPI);

void setup()
{
  pinMode(SS, OUTPUT);
  SPI.begin();
  Serial.begin(9600);
}

void loop()
{
  Serial.write("__LOOP__\r\n");
  auto status = bms.GetVoltageStatus();
  Serial.write("\r\n");
  if (status.has_value())
  {
    Serial.write("VOLTAGE:\r\n");
    Serial.write((std::to_string(status.value().sum / 10000) + "V\r\n").c_str());
    Serial.write("\r\n");
  }
  delay(1000);
}