
#include "bms_spi_utils.h"
#include "bms_udp_utils.h"
#include "bms_can_utils.h"
void setup()
{

  Serial.begin(board::UART_BITRATE);
  can::driver::setup();
  // udp::driver::setup();
  spi::ltc6811::driver::setup();
}

void loop()
{
  delay(1000);
  spi::ltc6811::driver::loop(); /*正確な測定をするために電圧回復を待つので，放電を行うときは，２回に１回放電をSTOPします．*/
  spi::ltc6811::data::dbg();
  // udp::driver::report(ltc6811::data::cell_data.sum, isl28022::data::current, ltc6811::data::cell_data, ltc6811::data::temp_data);
  can::csnv700::driver::loop();
  can::csnv700::data::dbg();
  can::driver::report(spi::ltc6811::data::cell_data.sum, can::csnv700::data::current, spi::ltc6811::data::cell_data, spi::ltc6811::data::temp_data);
}