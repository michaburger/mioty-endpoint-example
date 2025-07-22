# HAL library CMakeLists.txt

add_library(hal_lib STATIC
    # pico_hal.cpp  # Will be created when implementing HAL
)

target_include_directories(hal_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(hal_lib
    pico_stdlib
    hardware_gpio
    hardware_spi
    hardware_i2c
)
