# Application library CMakeLists.txt

add_library(app_lib STATIC
    application.cpp
)

target_include_directories(app_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(app_lib
    mioty_config
    mioty_drivers
    utils_lib
    pico_stdlib
    hardware_watchdog
    pico_unique_id
    pico_time
)
