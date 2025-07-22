/**
 * @file board_config.cpp
 * @brief Board-specific hardware configuration implementation
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#include "board_config.hpp"
#include "../../lib/utils/logger.hpp"
#include "hardware/gpio.h"

BoardConfig::BoardConfig() 
    : m_initialized(false)
    , m_led_state(false)
{
}

BoardConfig::~BoardConfig() {
    if (m_initialized) {
        // Cleanup if needed
    }
}

bool BoardConfig::initialize() {
    Logger::debug("Initializing board configuration");
    
    // Initialize status LED
    gpio_init(Board::LED_PIN);
    gpio_set_dir(Board::LED_PIN, GPIO_OUT);
    gpio_put(Board::LED_PIN, 0); // Start with LED off
    
    // Initialize other GPIO pins as needed
    // This is where we would initialize SPI pins, I2C pins, etc.
    
    m_initialized = true;
    Logger::info("Board configuration initialized successfully");
    
    return true;
}

void BoardConfig::setStatusLED(bool state) {
    if (m_initialized) {
        gpio_put(Board::LED_PIN, state);
        m_led_state = state;
    }
}

bool BoardConfig::getStatusLED() const {
    return m_led_state;
}
