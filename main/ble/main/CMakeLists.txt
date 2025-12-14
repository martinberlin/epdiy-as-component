set(app_sources "receive-ble.cpp")

idf_component_register(
    SRCS ${app_sources} 
    REQUIRES epdiy 
    nvs_flash
    esp_http_client # http client is used for NTP time sync
    bt      # Bluetooth
    jpegdec # Jpeg decoder
    bb_rtc  # RTC (RV3032 in v7_103)
    )
