/**
 * @file powerbank_keepalive.hpp
 * @brief Power bank keep-alive utility to prevent auto-shutoff
 * 
 * This module provides a periodic dummy load to prevent USB power banks
 * from automatically shutting off due to low current draw. Many power banks
 * turn off when current draw falls below ~60-100mA for extended periods.
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include "pico/stdlib.h"

namespace PowerBankKeepAlive {

/**
 * @class KeepAliveManager
 * @brief Manages periodic dummy load to keep power banks active
 * 
 * The class generates periodic current pulses by driving a GPIO pin high
 * for short durations. This prevents power banks from entering auto-shutoff
 * mode due to low current consumption.
 */
class KeepAliveManager {
public:
    /**
     * @brief Constructor
     */
    KeepAliveManager();
    
    /**
     * @brief Destructor
     */
    ~KeepAliveManager();
    
    /**
     * @brief Initialize the keep-alive system
     * @param load_gpio GPIO pin for dummy load
     * @param led_gpio GPIO pin for LED indicator (optional, use UINT8_MAX to disable)
     * @param pulse_interval_ms Interval between pulses in milliseconds
     * @param pulse_duration_ms Duration of each pulse in milliseconds
     * @param use_external_resistor true if external resistor is used, false for GPIO drive only
     * @return true if initialization successful, false otherwise
     */
    bool initialize(uint8_t load_gpio, uint8_t led_gpio, 
                   uint32_t pulse_interval_ms, uint32_t pulse_duration_ms,
                   bool use_external_resistor = true);
    
    /**
     * @brief Check if keep-alive system is enabled and initialized
     * @return true if active, false otherwise
     */
    bool isActive() const;
    
    /**
     * @brief Update the keep-alive system (call regularly in main loop)
     * This method checks timing and activates/deactivates the dummy load as needed
     */
    void update();
    
    /**
     * @brief Manually trigger a keep-alive pulse
     * Useful for triggering a pulse before long operations or sleep
     */
    void triggerPulse();
    
    /**
     * @brief Get statistics about keep-alive operation
     * @param total_pulses_out Total number of pulses generated
     * @param last_pulse_time_out Timestamp of last pulse (us since boot)
     */
    void getStatistics(uint32_t& total_pulses_out, uint64_t& last_pulse_time_out) const;
    
    /**
     * @brief Reset statistics counters
     */
    void resetStatistics();

private:
    // Configuration
    uint8_t m_load_gpio;
    uint8_t m_led_gpio;
    uint32_t m_pulse_interval_ms;
    uint32_t m_pulse_duration_ms;
    
    // State tracking
    bool m_initialized;
    bool m_pulse_active;
    uint64_t m_last_pulse_start_time;
    uint64_t m_next_pulse_time;
    
    // Statistics
    uint32_t m_total_pulses;
    uint64_t m_last_pulse_time;
    
    /**
     * @brief Start a dummy load pulse
     */
    void startPulse();
    
    /**
     * @brief Stop the current dummy load pulse
     */
    void stopPulse();
    
    /**
     * @brief Get current time in microseconds since boot
     * @return Time in microseconds
     */
    uint64_t getCurrentTimeMicros() const;
};

} // namespace PowerBankKeepAlive
