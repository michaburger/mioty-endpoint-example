/**
 * @file app_config.hpp
 * @brief Application-wide configuration settings
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "board_config.hpp"
#include "../../drivers/mioty/ts_unb_driver.hpp"

namespace Config {
    // Application version
    constexpr const char* APP_VERSION = "1.0.0";
    constexpr const char* APP_NAME = "Mioty End-Point Example";
    
    // Firmware version for payload header (1-byte each)
    constexpr uint8_t FIRMWARE_VERSION_MAJOR = 1;
    constexpr uint8_t FIRMWARE_VERSION_MINOR = 0;
    
    // Timing configurations
    constexpr uint32_t MAIN_LOOP_DELAY_MS = 1000;
    constexpr uint32_t LED_BLINK_DELAY_MS = 250;
    constexpr uint32_t WATCHDOG_TIMEOUT_MS = 8000;
    
    // Communication settings
    constexpr uint32_t UART_BAUD_RATE = 115200;
    constexpr bool ENABLE_DEBUG_OUTPUT = true;
    
    // Debug settings
    constexpr bool ENABLE_NETWORK_KEY_DEBUG = false;  // Print network key in debug output (disable for production!)
    
    // Sensor sampling rates (in milliseconds)
    constexpr uint32_t TEMPERATURE_SAMPLE_INTERVAL_MS = 20000;
    
    // Temperature sensor calibration
    // Linear offset to correct RP2040 internal temperature sensor readings
    // Adjust this value based on comparison with a reference thermometer
    // Positive values increase the reading, negative values decrease it
    // Example: If sensor reads 18.4°C but actual temperature is 25.7°C, 
    // set offset to 7.3°C to correct the reading
    constexpr float TEMPERATURE_CALIBRATION_OFFSET_C = 7.3f;
    
    // Mioty/TS-UNB settings
    constexpr bool ENABLE_MIOTY = true;
    constexpr uint32_t MIOTY_TRANSMISSION_INTERVAL_MS = 30000; // 30 seconds

    // Mioty/TS-UNB radio configuration
    namespace Mioty {
        // Network and device identification: 2b7e151628aed2a6abf7158809cf4f3c
        constexpr uint8_t NETWORK_KEY[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                           0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
        
        // Radio configuration using proper enum types
        constexpr TSUNBDriver::Region REGION = TSUNBDriver::Region::EU1;
        constexpr TSUNBDriver::ChipType CHIP_TYPE = TSUNBDriver::ChipType::RFM69HW;
        constexpr uint8_t TX_POWER_DBM = 14;                // TX power in dBm (max 14 for RFM69HW in EU)
        
        // Protocol configuration
        constexpr uint32_t INITIAL_EXT_PKG_CNT = 0;        // Extended packet counter initial value
        
        // Device identity configuration
        // Using static configuration for this specific sample node
        constexpr bool USE_BOARD_ID_FOR_EUI64 = true;     // Use static EUI64 configuration
        constexpr bool USE_BOARD_ID_FOR_SHORT_ADDR = true; // Use static short address configuration
        
        // Static configuration for this sample node
        constexpr uint8_t STATIC_EUI64[8] = {0x70, 0xB3, 0xD5, 0x67, 0x70, 0xFF, 0x00, 0x01};
        constexpr uint8_t STATIC_SHORT_ADDR[2] = {0x00, 0x01};
        
        // Region Details:
        // - EU0: Europe 868.0-868.6 MHz (older standard)
        // - EU1: Europe 868.7-869.2 MHz (recommended for new deployments)  
        // - EU2: Europe 869.4-869.65 MHz
        // - US0: US 902-928 MHz (915 MHz center)
        
        // Chip Type Details:
        // - RFM69W:  Standard power up to +13 dBm (20mW)
        // - RFM69HW: High power up to +20 dBm (100mW), but limited by regional regulations
    }
    
    // Power management
    constexpr bool ENABLE_SLEEP_MODE = false;
    constexpr uint32_t SLEEP_DURATION_MS = 30000;
    
    // Power bank compatibility feature
    // Enables a periodic dummy load to prevent USB power banks from auto-shutoff
    // due to low current draw (typically <60-100mA triggers shutoff)
    constexpr bool POWER_FROM_POWERBANK = true;
    
    // Dummy load configuration (when POWER_FROM_POWERBANK is true)
    namespace PowerBankKeepAlive {
        // Pulse load timing
        constexpr uint32_t PULSE_INTERVAL_MS = 3000;      // Pulse every 3 seconds
        constexpr uint32_t PULSE_DURATION_MS = 300;       // Pulse for 300ms

        // Load current configuration
        constexpr bool USE_EXTERNAL_RESISTOR = true;       // Use external resistor for higher current
        
        // Hardware connection options:
        // 1. USE_EXTERNAL_RESISTOR = true:
        //    Connect GPIO pin (Board::GPIO::POWERBANK_LOAD_PIN) through a resistor to GND
        //    For ~33mA at 3.3V: R = 3.3V / 0.033A = 100Ω (safe within 50mA GPIO limit)
        //    Use a 100Ω, 0.25W resistor for optimal safety
        //    Power dissipation during pulse: P = 3.3V * 0.033A = 0.109W
        //
        // 2. USE_EXTERNAL_RESISTOR = false:
        //    Use only GPIO pin drive current (~20-25mA at 12mA drive strength)
        //    Less effective but no external components required
        //    May not be sufficient for all power banks
        
        // LED indicator for dummy load activity (uses board LED)
        constexpr bool ENABLE_LOAD_LED_INDICATOR = false;
    }
}