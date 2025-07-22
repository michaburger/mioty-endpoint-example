# Mioty/TS-UNB drivers CMakeLists.txt

add_library(mioty_drivers STATIC
    ts_unb_driver.cpp
)

target_include_directories(mioty_drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mioty_drivers
    common_drivers
    ts_unb_lib_rfm69
    pico_stdlib
    hardware_spi
    hardware_gpio
)
