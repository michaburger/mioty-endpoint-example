# Utilities library CMakeLists.txt

add_library(utils_lib STATIC
    logger.cpp
    powerbank_keepalive.cpp
)

target_include_directories(utils_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(utils_lib
    pico_stdlib
    pico_time
    hardware_gpio
)
