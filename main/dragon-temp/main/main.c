/* Simple firmware for a ESP32 displaying a static image on an EPaper Screen */

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "dragon.h"
#include "epd_highlevel.h"
#include "epdiy.h"
#include <driver/i2c.h>
#include <driver/gpio.h>
#include "board/tps65185.h"
EpdiyHighlevelState hl;

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
    }
}


void idf_loop() {
    EpdRect dragon_area = { .x = 0, .y = 0, .width = dragon_width, .height = dragon_height };

    int temperature = 25;

    
    epd_fullclear(&hl, temperature);

    epd_copy_to_framebuffer(dragon_area, dragon_data, epd_hl_get_framebuffer(&hl));

    enum EpdDrawError _err = epd_hl_update_screen(&hl, MODE_GC16, temperature);
    
    
    vTaskDelay(1000);
}

void idf_setup() {
    epd_init(&epd_board_v7, &ED097TC2, EPD_LUT_64K);
    epd_set_vcom(1560);
    hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
    epd_poweron();
    gpio_set_direction(IO_HIGH_TEMP, GPIO_MODE_OUTPUT);
    // Turn OFF, on 0 gives GND to LED
    gpio_set_level(IO_HIGH_TEMP, 1);

    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;
    int STACK_SIZE = 4024;

    // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
    // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
    // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
    // the new task attempts to access it.
    xTaskCreate( vTaskCheckTemperature, "CheckTemperature", STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
    configASSERT( xHandle );
}

#ifndef ARDUINO_ARCH_ESP32
void app_main() {
    idf_setup();

    while (1) {
        idf_loop();
    };
}
#endif
