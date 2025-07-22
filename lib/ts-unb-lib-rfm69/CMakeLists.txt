# TS-UNB-Lib CMakeLists.txt
# Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

add_library(ts_unb_lib_rfm69 
    src/RPPicoTsUnb_globals.cpp
)

target_include_directories(ts_unb_lib_rfm69 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/TsUnb
    ${CMAKE_CURRENT_SOURCE_DIR}/Trx
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Encryption
)

target_link_libraries(ts_unb_lib_rfm69 PUBLIC
    pico_stdlib
    hardware_spi
    hardware_gpio
    hardware_irq
)

# Add required compile definitions
target_compile_definitions(ts_unb_lib_rfm69 PUBLIC
    # Add any required definitions here
)
