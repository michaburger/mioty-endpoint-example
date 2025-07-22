/**
 * @file board_config.hpp
 * @brief Board-specific hardware configuration
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "pico/stdlib.h"

namespace Board {
    // Hardware version for payload header (1-byte)
    constexpr uint8_t HARDWARE_VERSION = 1;
    
    // LED configuration
    #if defined(PICO_DEFAULT_LED_PIN)
        constexpr uint LED_PIN = PICO_DEFAULT_LED_PIN;
    #else
        constexpr uint LED_PIN = 25; // Default for Pico
    #endif
    
    // GPIO pin assignments
    namespace GPIO {
        // SPI pins for Mioty module (SPI0)
        constexpr uint MIOTY_SPI_SCK  = 18;  // SCK: GP18
        constexpr uint MIOTY_SPI_MOSI = 19;  // TX: GP19
        constexpr uint MIOTY_SPI_MISO = 16;  // RX: GP16
        constexpr uint MIOTY_SPI_CS   = 17;  // CSn: GP17
        constexpr uint MIOTY_RESET    = 22;  // GP22 for Reset pin
        
        // I2C pins for sensors
        // I2C0 interface
        constexpr uint I2C0_SDA = 4;  // GP4
        constexpr uint I2C0_SCL = 5;  // GP5
        
        // I2C1 interface
        constexpr uint I2C1_SDA = 6;  // GP6
        constexpr uint I2C1_SCL = 7;  // GP7
    }
    
    // Communication interfaces
    namespace Comm {
        // SPI interface for Mioty module
        #define MIOTY_SPI_INTERFACE spi0
        constexpr uint MIOTY_SPI_BAUDRATE = 4000000; // 4 MHz for Mioty RFM69HW
        
        // I2C interfaces
        #define I2C0_INTERFACE i2c0
        #define I2C1_INTERFACE i2c1
        constexpr uint I2C_BAUDRATE = 400000;  // 400 kHz
    }
    
    // Power management
    namespace Power {
        constexpr float BATTERY_LOW_VOLTAGE = 3.2f;
        constexpr float BATTERY_CRITICAL_VOLTAGE = 3.0f;
    }
}

/**
 * @brief Board configuration management class
 */
class BoardConfig {
public:
    BoardConfig();
    ~BoardConfig();
    
    /**
     * @brief Initialize board hardware
     * @return true if successful
     */
    bool initialize();
    
    /**
     * @brief Set status LED state
     * @param state true to turn on, false to turn off
     */
    void setStatusLED(bool state);
    
    /**
     * @brief Get status LED state
     * @return true if LED is on, false if off
     */
    bool getStatusLED() const;

private:
    bool m_initialized;
    bool m_led_state;
};
