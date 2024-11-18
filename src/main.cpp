
#include "bms_spi_utils.h"
#include "bms_udp_utils.h"
#include "bms_can_utils.h"
#include "bms_soc_utils.h"

SemaphoreHandle_t ltc6811_data_semaphore;
SemaphoreHandle_t csnv700_data_semaphore;
SemaphoreHandle_t soc_data_semaphore;

void can_send_task(void *pvParameters)
{
  for (;;)
  {
    xSemaphoreTake(ltc6811_data_semaphore, portMAX_DELAY);
    xSemaphoreTake(csnv700_data_semaphore, portMAX_DELAY);
    xSemaphoreTake(soc_data_semaphore, portMAX_DELAY);
    can::driver::report(spi::ltc6811::data::cell_data.sum, can::csnv700::data::current, soc::param::FULL_CAPACITY, soc::data::soc, spi::ltc6811::data::cell_data, spi::ltc6811::data::temp_data);
    xSemaphoreGive(soc_data_semaphore);
    xSemaphoreGive(csnv700_data_semaphore);
    xSemaphoreGive(ltc6811_data_semaphore);
    vTaskDelay(500);
  }
}

void udp_send_task(void *pvParameters)
{
  for (;;)
  {
    xSemaphoreTake(ltc6811_data_semaphore, portMAX_DELAY);
    xSemaphoreTake(csnv700_data_semaphore, portMAX_DELAY);
    xSemaphoreTake(soc_data_semaphore, portMAX_DELAY);
    udp::driver::report(spi::ltc6811::data::cell_data.sum, can::csnv700::data::current, spi::ltc6811::data::cell_data, spi::ltc6811::data::temp_data);
    xSemaphoreGive(soc_data_semaphore);
    xSemaphoreGive(csnv700_data_semaphore);
    xSemaphoreGive(ltc6811_data_semaphore);
    vTaskDelay(2000);
  }
}

void csnv700_task(void *pvParameters)
{
  QueueHandle_t xQueue;
  xQueue = (QueueHandle_t)pvParameters;
  uint8_t buffer[8];
  for (;;)
  {
    xSemaphoreTake(csnv700_data_semaphore, portMAX_DELAY);
    BaseType_t result = xQueueReceive(xQueue, buffer, portMAX_DELAY);
    if (result == pdPASS)
    {
      auto response = can::csnv700::driver::Response(buffer);
      if (response.check_crc())
      {
        response.parse();
        soc::driver::update_soc(can::csnv700::data::current, spi::ltc6811::data::cell_data.sum / 10000);
      }
    }
    xSemaphoreGive(csnv700_data_semaphore);
    vTaskDelay(1);
  }
}
void ltc6811_task(void *pvParameters)
{
  for (;;)
  {
    if (xSemaphoreTake(ltc6811_data_semaphore, 0) == pdTRUE)
    {
      spi::ltc6811::driver::loop();
      xSemaphoreGive(ltc6811_data_semaphore);
      vTaskDelay(1000);
    }
  }
}
void dbg_task(void *pvParameters)
{
  for (;;)
  {
    if (xSemaphoreTake(ltc6811_data_semaphore, 0) == pdTRUE)
    {
      // spi::ltc6811::data::dbg();
      xSemaphoreGive(ltc6811_data_semaphore);
    }
    if (xSemaphoreTake(csnv700_data_semaphore, portMAX_DELAY) == pdTRUE)
    {
      can::csnv700::data::dbg();
      xSemaphoreGive(csnv700_data_semaphore);
    }
    if (xSemaphoreTake(soc_data_semaphore, portMAX_DELAY) == pdTRUE)
    {
      soc::data::dbg();
      xSemaphoreGive(soc_data_semaphore);
    }

    vTaskDelay(1000);
  }
}

void setup()
{
  Serial.begin(board::UART_BITRATE);
  ltc6811_data_semaphore = xSemaphoreCreateBinary();
  csnv700_data_semaphore = xSemaphoreCreateBinary();
  soc_data_semaphore = xSemaphoreCreateBinary();
  can::driver::setup();
  // udp::driver::setup();
  spi::ltc6811::driver::setup();
  delay(1000);
  xTaskCreate(reinterpret_cast<TaskFunction_t>(ltc6811_task), "ltc6811", 2048, NULL, 1, NULL);
  xTaskCreate(reinterpret_cast<TaskFunction_t>(csnv700_task), "csnv700", 2048, can::driver::can_message_queue, 1, NULL);
  // // xTaskCreate(reinterpret_cast<TaskFunction_t>(udp_send_task), "udp_send", 2048, NULL, 1, NULL);
  xTaskCreate(reinterpret_cast<TaskFunction_t>(can_send_task), "can_send", 2048, NULL, 1, NULL);
  xTaskCreate(dbg_task, "dbg", 10000, NULL, 1, NULL);
  xSemaphoreGive(ltc6811_data_semaphore);
  xSemaphoreGive(csnv700_data_semaphore);
  xSemaphoreGive(soc_data_semaphore);
}

void loop()
{
  vTaskDelay(1000);
}