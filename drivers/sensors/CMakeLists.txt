# Sensor drivers CMakeLists.txt

add_library(sensor_drivers STATIC
    temperature/rp2040_temp_sensor.cpp    # RP2040 internal temperature sensor
    # humidity/dht22.cpp         # To be implemented
)

target_include_directories(sensor_drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sensor_drivers
    common_drivers
    pico_stdlib
    hardware_gpio
    hardware_adc
)
