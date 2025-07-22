# Common drivers CMakeLists.txt

add_library(common_drivers STATIC
    # gpio_manager.cpp      # To be implemented
    # spi_manager.cpp       # To be implemented
    # i2c_manager.cpp       # To be implemented
)

target_include_directories(common_drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(common_drivers
    pico_stdlib
    hardware_gpio
    hardware_spi
    hardware_i2c
)
