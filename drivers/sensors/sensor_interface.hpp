/**
 * @file sensor_interface.hpp
 * @brief Common interface for all sensor drivers
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>

/**
 * @brief Sensor status enumeration
 */
enum class SensorStatus {
    OK = 0,
    ERROR_NOT_INITIALIZED,
    ERROR_COMMUNICATION,
    ERROR_TIMEOUT,
    ERROR_INVALID_DATA,
    ERROR_HARDWARE_FAULT
};

/**
 * @brief Base interface for all sensor drivers
 */
class SensorInterface {
public:
    virtual ~SensorInterface() = default;
    
    /**
     * @brief Initialize the sensor
     * @return SensorStatus indicating success or failure
     */
    virtual SensorStatus initialize() = 0;
    
    /**
     * @brief Check if sensor is initialized and ready
     * @return true if ready, false otherwise
     */
    virtual bool isReady() const = 0;
    
    /**
     * @brief Get sensor name/identifier
     * @return Sensor name string
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Read sensor data
     * @return SensorStatus indicating success or failure
     */
    virtual SensorStatus read() = 0;
    
    /**
     * @brief Get the last error status
     * @return Last error status
     */
    virtual SensorStatus getLastError() const = 0;
    
    /**
     * @brief Reset the sensor
     * @return SensorStatus indicating success or failure
     */
    virtual SensorStatus reset() = 0;

protected:
    SensorStatus m_last_error = SensorStatus::ERROR_NOT_INITIALIZED;
    bool m_initialized = false;
};
