/* Simple firmware for a ESP32 displaying a static image on an EPaper Screen */

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "epd_highlevel.h"
#include "epdiy.h"
#include "board/tps65185.h"

EpdiyHighlevelState hl;

int temperature = 25;

float MAX_TEMP = 29.0;
bool powered = true;
#define IO_HIGH_TEMP GPIO_NUM_1

// Task running all the time in the background
void vTaskCheckTemperature( void * pvParameters )
{
  float temp;
  for( ;; )
    {
      // Read temperature only if TPS is powered will work
      if (powered) {
        temp = tps_read_thermistor(I2C_NUM_0);
      } else {
        temp = 0;
      }
      
      if (temp > MAX_TEMP) {
          gpio_set_level(IO_HIGH_TEMP, 0);
          ESP_LOGW("WARNING", "Temperature overheat (Major than %.1f)", MAX_TEMP);
          powered = false;
          epd_poweroff();
      } else {
        gpio_set_level(IO_HIGH_TEMP, 1);
        ESP_LOGI("TPS temp", "%.1f °C power:%d", temp, (int)powered);
      }

      vTaskDelay(pdMS_TO_TICKS(500));
    } // Keep looping forever
}

void idf_loop() {
    // make a full black | white print to force epdiy to send the update
    epd_fill_rect(epd_full_screen(), 0, epd_hl_get_framebuffer(&hl));
    epd_hl_update_screen(&hl, MODE_DU, temperature);
    vTaskDelay(pdMS_TO_TICKS(1));
    epd_fill_rect(epd_full_screen(), 255, epd_hl_get_framebuffer(&hl));
    epd_hl_update_screen(&hl, MODE_DU, temperature);
}

void idf_setup() {
    epd_init(&epd_board_v7, &ED133UT2, EPD_LUT_64K);
    
    gpio_set_direction(IO_HIGH_TEMP, GPIO_MODE_OUTPUT);
    // Turn OFF, on 0 gives GND to LED
    gpio_set_level(IO_HIGH_TEMP, 1);

    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;
    int STACK_SIZE = 4024;
    
    // SET WAVEFORM
    // NOTE: For me gives more consistent results with a normal waveform
    hl = epd_hl_init(&epdiy_ED097TC2);
    //                epdiy_NULL for the 0,0,0 (no action waveform)
    
    // starts the board in kickback more
    tps_vcom_kickback();


    // Start check Temperature task
    xTaskCreate( vTaskCheckTemperature, "CheckTemperature", STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
    configASSERT( xHandle );

    // display starts to pass BLACK to WHITE but doing nothing+
    // dince the NULL waveform is full of 0 "Do nothing for each pixel"
    idf_loop();
    // start measure and set ACQ bit:
    tps_vcom_kickback_start();
    int isrdy = 1;
    int kickback_volt = 0;
    while (kickback_volt == 0) {
        idf_loop();
        isrdy++;
        if (powered) {
         kickback_volt = tps_vcom_kickback_rdy();
        } else {
          // If unpowered, let's keep LED on since there was a HIGH temp detected
          gpio_set_level(IO_HIGH_TEMP, 0);
        }
    }
    if (powered) {
      ESP_LOGI("vcom", "readings are of %d mV. It was ready in %d refreshes", kickback_volt, isrdy);
    } else {
      ESP_LOGI("POWER LINES", "Down now. Overheat detected");
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();
}

#ifndef ARDUINO_ARCH_ESP32
void app_main() {
    idf_setup();
}
#endif
