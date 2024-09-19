

# Firmware Document

## Example Code in main.cpp

```c++

#include "bms_can_utils.h"
#include "bms_udp_utils.h"
#include "bms_ltc6811_driver.h"
#include "bms_isl28022_driver.h"

void setup() {
    can::driver::setup();
    udp::driver::setup();
    ltc6811::driver::setup();
    isl28022::driver::setup();
}

void loop() {
    delay(1000);
    ltc6811::driver::loop();
    ltc6811::data::dbg(); // Serial Print Debug
    isl28022::driver::loop();
    isl28022::data::dbg(); // Serial Print Debug
    udp::driver::report(ltc6811::data::cell_data.sum, isl28022::data::current, ltc6811::data::cell_data, ltc6811::data::temp_data);
    can::driver::report(ltc6811::data::cell_data.sum, isl28022::data::current, ltc6811::data::cell_data, ltc6811::data::temp_data);
}
```

## Communication Protocol on CAN/UDP
![](./canprotocol_preview.svg)


## Various parameter adjustments ```src/params.h```

| Variables         | Meaning                                          |
| ----------------- | ------------------------------------------------ |
| CAN_ID            | Board-specific CANID                             |
| CHANE_LENGTH      | Number of ltc6811s connected to the master board |
| CAN_BITRATE       | canbus bitrate                                   |
| HOST_IP_ADDRESS   | Destination address of UDP packets               |
| DEVICE_IP_ADDRESS | Board-specific IP address                        |
| GATEWAY_ADDRESS   | gateway address                                  |
| SUBNET_MASK       | subnet mask                                      |
| SSID              | wifi ssid                                        |
| PASSWORD          | wifi password                                    |

## CAN Driver ```src/bms_can_utils.h```

```c++
namespace driver {

/**
* @brief Initializes the CAN (Controller Area Network) communication.
* 
* This function sets the CAN pins, starts the CAN bus with a specified bitrate,
* sets up a filter for extended CAN messages, defines a callback function for
* received messages, and sets a timeout.
* 
* @return false if the initialization is successful, true if it fails.
*/
    bool setup() {}
/**
 * @brief Reports various BMS (Battery Management System) status data over CAN.
 * 
 * This function sends multiple CAN messages containing the voltage, current, cell voltage range,
 * throttle discharge status, and temperature data. If a request flag is set, it also sends detailed
 * cell voltage data.
 * 
 * @param voltage The voltage value to report.
 * @param current The current value to report.
 * @param cell_data The cell voltage data to report.
 * @param temp_data The temperature data to report.
 * @return true Always returns true after sending the data.
 */
    bool report(uint32_t voltage, uint32_t current, ltc6811::data::CellVoltage &cell_data, ltc6811::data::Temperature &temp_data)
};
```

## UDP Driver ```src/bms_udp_utils.h```
```c++
/**
 * @brief Sets up the WiFi and UDP communication.
 * 
 * This function configures the WiFi with a static IP address, sets the WiFi mode to station,
 * connects to the specified WiFi network, and starts listening for UDP packets on port 12351.
 * When a UDP packet is received, the `response_cellvol` callback function is called.
 */
void setup() {}

/**
 * @brief Reports various BMS (Battery Management System) status data over UDP.
 * 
 * This function sends multiple UDP messages containing the voltage, current, cell voltage range,
 * throttle discharge status, and temperature data. If a request flag is set, it also sends detailed
 * cell voltage data.
 * 
 * @param voltage The voltage value to report.
 * @param current The current value to report.
 * @param cell_data The cell voltage data to report.
 * @param temp_data The temperature data to report.
 * @return true Always returns true after sending the data.
 */
bool report(uint32_t voltage, uint32_t current, ltc6811::data::CellVoltage &cell_data, ltc6811::data::Temperature &temp_data)
{}

```

## BMS(LTC6811) Driver ```src/bms_ltc6811_driver.h```

## BMS(ISL28022) Driver ```src/bms_isl28022_driver.h```



