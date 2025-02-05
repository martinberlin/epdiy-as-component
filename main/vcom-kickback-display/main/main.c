/* Simple firmware for a ESP32 displaying a static image on an EPaper Screen */

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "dragon.h"
#include "epd_highlevel.h"
#include "epdiy.h"
#include "board/tps65185.h"
// Add display
#include "ssd1306.h"
#include "font8x8_basic.h"
SSD1306_t dev;

EpdiyHighlevelState hl;

// choose the default demo board depending on the architecture
#ifdef CONFIG_IDF_TARGET_ESP32
#define DEMO_BOARD epd_board_v6
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define DEMO_BOARD epd_board_v7
#endif
int temperature = 25;

void draw_dragon() {
    EpdRect dragon_area = { .x = 0, .y = 0, .width = dragon_width, .height = dragon_height };
    epd_poweron();
    epd_fullclear(&hl, temperature);
    epd_copy_to_framebuffer(dragon_area, dragon_data, epd_hl_get_framebuffer(&hl));
    enum EpdDrawError _err = epd_hl_update_screen(&hl, MODE_GC16, temperature);
    epd_poweroff();
}

void idf_loop() {
    // make a full black | white print to force epdiy to send the update
    epd_fill_rect(epd_full_screen(), 0, epd_hl_get_framebuffer(&hl));
    epd_hl_update_screen(&hl, MODE_DU, temperature);
    vTaskDelay(pdMS_TO_TICKS(1));
    epd_fill_rect(epd_full_screen(), 255, epd_hl_get_framebuffer(&hl));
    epd_hl_update_screen(&hl, MODE_DU, temperature);
}

void ssd_init() {
    #if CONFIG_I2C_INTERFACE
        ESP_LOGI("OLED", "INTERFACE is i2c");
        ESP_LOGI("OLED", "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
        ESP_LOGI("OLED", "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
        i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    #endif
    #if CONFIG_SSD1306_128x64
        ESP_LOGI("OLED", "Panel is 128x64");
        ssd1306_init(&dev, 128, 64);
    #endif // CONFIG_SSD1306_128x64
    #if CONFIG_SSD1306_128x32
        ESP_LOGI("OLED", "Panel is 128x32");
        ssd1306_init(&dev, 128, 32);
    #endif // CONFIG_SSD1306_128x32

	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text_x3(&dev, 0, "VCOM", 5, false);
}

void ssd_text(char * text, int len, bool invert, int vcom) {
    //ssd1306_display_text_x3(&dev, 1, text, len, true); // Bigger
    ssd1306_display_text(&dev, 3,  text, len, invert);
    if (vcom == 0) {
        ssd1306_display_text(&dev, 4,  "DISCONNECTED", 12, true);
    } else if (vcom >= 100 && vcom <=1000) {
        ssd1306_display_text(&dev, 4,  "OK", 2, invert);
    } else {
        ssd1306_display_text(&dev, 4,  "BROKEN", 5, true);
    }
}


void idf_setup() {
    // Initialize display
    ssd_init();
    epd_init(&DEMO_BOARD, &ED133UT2, EPD_LUT_64K);

    // SET WAVEFORM
    // NOTE: For me gives more consistent results with a normal waveform
    hl = epd_hl_init(&epdiy_NULL);
    //                epdiy_NULL for the 0,0,0 (no action waveform)
    
    // starts the board in kickback more
    tps_vcom_kickback();

    // display starts to pass BLACK to WHITE but doing nothing+
    // dince the NULL waveform is full of 0 "Do nothing for each pixel"
    idf_loop();
    // start measure and set ACQ bit:
    tps_vcom_kickback_start();
    char buffer[100];
    int isrdy = 1;
    int kickback_volt = 0;
    while (kickback_volt == 0) {
        idf_loop();
        isrdy++;
        kickback_volt = tps_vcom_kickback_rdy();
        sprintf(buffer, "%d mV", kickback_volt);
        ssd_text(buffer, sizeof(buffer), false, kickback_volt);
    }
    
    ESP_LOGI("vcom", "readings are of %d mV. It was ready in %d refreshes", kickback_volt, isrdy);
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();
}

#ifndef ARDUINO_ARCH_ESP32
void app_main() {
    idf_setup();
}
#endif
