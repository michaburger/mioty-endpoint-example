# Config library CMakeLists.txt

add_library(mioty_config
    board_config.cpp
    payload_config.cpp
)

target_include_directories(mioty_config PUBLIC
    .
    ..
)

target_link_libraries(mioty_config
    utils_lib
    pico_stdlib
    hardware_gpio
)
