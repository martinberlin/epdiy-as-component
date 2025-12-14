# Uncomment only one app_source
idf_build_get_property(target IDF_TARGET)

# 1 Example for IT8951 Boards (Adjusted for DEXA-C097 Good-Display)
message(STATUS "→ Building example for ${target}")

#set(EXTRA_COMPONENT_DIRS components/ssd1306)
# Select one source example to build
#set(app_sources "dragon-temp/main/main.c") # Just to draw something

set(app_sources "ble/main/receive-ble.cpp")
# 1 Only VCOM kickback reading via Serial output: idf.py monitor
#set(app_sources "vcom-kickback/main/main.c")

# 2 VCOM kickback reading + output to I2C display ssd1306
#set(app_sources "vcom-kickback-display/main/main.c")

set(app_requires 
                epdiy
                bb_rtc
                ssd1306
                nvs_flash
                bt
                esp_http_client
                jpegdec
                )

idf_component_register(SRCS ${app_sources}
        REQUIRES ${app_requires}
        )
