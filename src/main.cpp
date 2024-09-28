
#include "bms_ltc6811_driver.h"
#include "bms_isl28022_driver.h"
#include "bms_udp_utils.h"
#include "bms_can_utils.h"
void setup()
{

  Serial.begin(board::UART_BITRATE);
  // can::driver::setup();
  // udp::driver::setup();
  ltc6811::driver::setup();
  isl28022::driver::setup();
}

void loop()
{
  delay(1000);
  ltc6811::driver::loop();
  // ltc6811::data::dbg();
  // isl28022::driver::loop();
  // isl28022::data::dbg();
  // udp::driver::report(ltc6811::data::cell_data.sum, isl28022::data::current, ltc6811::data::cell_data, ltc6811::data::temp_data);
  // can::driver::report(ltc6811::data::cell_data.sum, isl28022::data::current, ltc6811::data::cell_data, ltc6811::data::temp_data);
}