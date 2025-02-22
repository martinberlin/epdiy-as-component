# Uncomment only one app_source
idf_build_get_property(target IDF_TARGET)

# 1 Example for IT8951 Boards (Adjusted for DEXA-C097 Good-Display)
message(STATUS "→ Building example for ${target}")

set(EXTRA_COMPONENT_DIRS components/ssd1306)
# Select one source example to build
set(app_sources "dragon-temp/main/main.c") # Just to draw something

# 1 Only VCOM kickback reading via Serial output: idf.py monitor
#set(app_sources "vcom-kickback/main/main.c")

# 2 VCOM kickback reading + output to I2C display ssd1306
#set(app_sources "vcom-kickback-display/main/main.c")

set(app_requires 
                epdiy
                ssd1306
                )

idf_component_register(SRCS ${app_sources}
        REQUIRES ${app_requires}
        )
