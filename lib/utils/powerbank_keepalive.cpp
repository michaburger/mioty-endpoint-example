/**
 * @file powerbank_keepalive.cpp
 * @brief Power bank keep-alive utility implementation
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#include "powerbank_keepalive.hpp"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "logger.hpp"

namespace PowerBankKeepAlive {

KeepAliveManager::KeepAliveManager()
    : m_load_gpio(UINT8_MAX)
    , m_led_gpio(UINT8_MAX)
    , m_pulse_interval_ms(5000)
    , m_pulse_duration_ms(150)
    , m_initialized(false)
    , m_pulse_active(false)
    , m_last_pulse_start_time(0)
    , m_next_pulse_time(0)
    , m_total_pulses(0)
    , m_last_pulse_time(0) {
}

KeepAliveManager::~KeepAliveManager() {
    if (m_initialized && m_pulse_active) {
        stopPulse();
    }
}

bool KeepAliveManager::initialize(uint8_t load_gpio, uint8_t led_gpio, 
                                 uint32_t pulse_interval_ms, uint32_t pulse_duration_ms,
                                 bool use_external_resistor) {
    
    if (pulse_interval_ms < pulse_duration_ms) {
        Logger::error("KeepAlive", "Pulse interval must be longer than pulse duration");
        return false;
    }
    
    if (pulse_duration_ms < 50) {
        Logger::warning("KeepAlive", "Very short pulse duration may not be effective");
    }
    
    m_load_gpio = load_gpio;
    m_led_gpio = led_gpio;
    m_pulse_interval_ms = pulse_interval_ms;
    m_pulse_duration_ms = pulse_duration_ms;
    
    // Initialize load GPIO as output, initially low
    gpio_init(m_load_gpio);
    gpio_set_dir(m_load_gpio, GPIO_OUT);
    gpio_put(m_load_gpio, false);
    
    // Configure GPIO drive strength based on external resistor usage
    if (use_external_resistor) {
        // With external resistor, use moderate drive strength to avoid overcurrent
        gpio_set_drive_strength(m_load_gpio, GPIO_DRIVE_STRENGTH_4MA);
        Logger::debug("KeepAlive", "Configured for external resistor mode (4mA drive)");
    } else {
        // Without external resistor, use maximum drive strength for higher current
        gpio_set_drive_strength(m_load_gpio, GPIO_DRIVE_STRENGTH_12MA);
        Logger::debug("KeepAlive", "Configured for internal drive mode (12mA drive)");
    }
    
    // Initialize LED GPIO if specified
    if (m_led_gpio != UINT8_MAX) {
        gpio_init(m_led_gpio);
        gpio_set_dir(m_led_gpio, GPIO_OUT);
        gpio_put(m_led_gpio, false);
    }
    
    // Set up timing
    uint64_t current_time = getCurrentTimeMicros();
    m_next_pulse_time = current_time + (m_pulse_interval_ms * 1000ULL);
    m_pulse_active = false;
    m_initialized = true;
    
    Logger::info("KeepAlive", "Initialized - Load GPIO: %d, LED GPIO: %d, Interval: %dms, Duration: %dms, External resistor: %s", 
                 m_load_gpio, 
                 (m_led_gpio == UINT8_MAX) ? -1 : m_led_gpio,
                 m_pulse_interval_ms, 
                 m_pulse_duration_ms,
                 use_external_resistor ? "Yes" : "No");
    
    return true;
}

bool KeepAliveManager::isActive() const {
    return m_initialized;
}

void KeepAliveManager::update() {
    if (!m_initialized) {
        return;
    }
    
    uint64_t current_time = getCurrentTimeMicros();
    
    if (m_pulse_active) {
        // Check if pulse duration has elapsed
        uint64_t pulse_elapsed = current_time - m_last_pulse_start_time;
        if (pulse_elapsed >= (m_pulse_duration_ms * 1000ULL)) {
            stopPulse();
            // Schedule next pulse
            m_next_pulse_time = current_time + (m_pulse_interval_ms * 1000ULL);
        }
    } else {
        // Check if it's time for the next pulse
        if (current_time >= m_next_pulse_time) {
            startPulse();
        }
    }
}

void KeepAliveManager::triggerPulse() {
    if (!m_initialized) {
        return;
    }
    
    // Stop current pulse if active
    if (m_pulse_active) {
        stopPulse();
    }
    
    // Start immediate pulse
    startPulse();
    
    Logger::debug("KeepAlive", "Manual pulse triggered");
}

void KeepAliveManager::getStatistics(uint32_t& total_pulses_out, uint64_t& last_pulse_time_out) const {
    total_pulses_out = m_total_pulses;
    last_pulse_time_out = m_last_pulse_time;
}

void KeepAliveManager::resetStatistics() {
    m_total_pulses = 0;
    m_last_pulse_time = 0;
    Logger::info("KeepAlive", "Statistics reset");
}

void KeepAliveManager::startPulse() {
    if (!m_initialized || m_pulse_active) {
        return;
    }
    
    // Activate dummy load
    gpio_put(m_load_gpio, true);
    
    // Activate LED indicator if configured
    if (m_led_gpio != UINT8_MAX) {
        gpio_put(m_led_gpio, true);
    }
    
    m_pulse_active = true;
    m_last_pulse_start_time = getCurrentTimeMicros();
    m_total_pulses++;
    m_last_pulse_time = m_last_pulse_start_time;
    
    Logger::debug("KeepAlive", "Pulse started (#%d)", m_total_pulses);
}

void KeepAliveManager::stopPulse() {
    if (!m_initialized || !m_pulse_active) {
        return;
    }
    
    // Deactivate dummy load
    gpio_put(m_load_gpio, false);
    
    // Deactivate LED indicator if configured
    if (m_led_gpio != UINT8_MAX) {
        gpio_put(m_led_gpio, false);
    }
    
    m_pulse_active = false;
    
    uint64_t pulse_duration = getCurrentTimeMicros() - m_last_pulse_start_time;
    Logger::debug("KeepAlive", "Pulse stopped (duration: %llu us)", pulse_duration);
}

uint64_t KeepAliveManager::getCurrentTimeMicros() const {
    return to_us_since_boot(get_absolute_time());
}

} // namespace PowerBankKeepAlive
