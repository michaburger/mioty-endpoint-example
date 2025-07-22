# Mioty Generic Node Framework - Main CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set compiler flags for better code quality and debugging
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(mioty_generic_node C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add subdirectories for modular build
add_subdirectory(lib)
add_subdirectory(drivers)
add_subdirectory(src)

# Create main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Link libraries in dependency order
target_link_libraries(${PROJECT_NAME}
    app_lib
    mioty_config
    mioty_drivers
    sensor_drivers
    common_drivers
    utils_lib
    hal_lib
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_gpio
    hardware_adc
)

# Add Pico W support if available
if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${PROJECT_NAME} pico_cyw43_arch_none)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)

# Create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})

# Optional: Enable USB output for debugging
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# add url via pico_set_program_url
