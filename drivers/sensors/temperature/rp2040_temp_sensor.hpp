/**
 * @file rp2040_temp_sensor.hpp
 * @brief RP2040 internal temperature sensor driver using built-in ADC
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../sensor_interface.hpp"
#include <string>
#include <cstdint>

/**
 * @brief RP2040 internal temperature sensor driver using built-in ADC
 * 
 * This driver uses the Raspberry Pi Pico's built-in temperature sensor connected to ADC channel 4.
 * The sensor provides die temperature readings with typical accuracy of ±2°C.
 * 
 * Features:
 * - No external components required
 * - Multi-sample averaging for better accuracy
 * - Built-in calibration for voltage-to-temperature conversion
 * - Standard SensorInterface compliance
 */
class RP2040TempSensor : public SensorInterface {
public:
    RP2040TempSensor();
    ~RP2040TempSensor() override;
    
    // SensorInterface implementation
    SensorStatus initialize() override;
    bool isReady() const override;
    std::string getName() const override;
    SensorStatus read() override;
    SensorStatus getLastError() const override;
    SensorStatus reset() override;
    
    /**
     * @brief Get the last temperature reading in Celsius
     * @return Temperature in degrees Celsius
     */
    float getTemperatureCelsius() const;
    
    /**
     * @brief Get the last temperature reading in Fahrenheit
     * @return Temperature in degrees Fahrenheit
     */
    float getTemperatureFahrenheit() const;

private:
    float m_last_temperature;
    uint64_t m_last_read_time;
    SensorStatus m_last_error;
    bool m_initialized;
    
    /**
     * @brief Read raw ADC value from internal temperature sensor
     * @return Raw ADC reading
     */
    uint16_t readInternalTempRaw();
    
    /**
     * @brief Convert raw ADC value to temperature in Celsius
     * @param raw_adc Raw ADC reading
     * @return Temperature in degrees Celsius
     */
    float convertRawToTemperature(uint16_t raw_adc);
};
