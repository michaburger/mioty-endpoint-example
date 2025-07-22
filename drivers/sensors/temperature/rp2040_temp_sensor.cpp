/**
 * @file rp2040_temp_sensor.cpp
 * @brief RP2040 internal temperature sensor driver implementation
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#include "rp2040_temp_sensor.hpp"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include <cmath>
#include "../../src/config/app_config.hpp"

// Temperature sensor is on ADC input 4
constexpr uint ADC_TEMP_CHANNEL = 4;

// Conversion constants for RP2040 temperature sensor
// These values are from the official Raspberry Pi Pico SDK and datasheet
constexpr float CONVERSION_FACTOR = 3.3f / (1 << 12); // 12-bit ADC, 3.3V ref
// Official calibration constants from Pico SDK
constexpr float TEMP_SLOPE = -0.001721f; // V/°C 
constexpr float TEMP_BIAS = 0.706f; // V at 27°C
constexpr float TEMP_OFFSET = 27.0f; // °C

RP2040TempSensor::RP2040TempSensor() 
    : m_last_temperature(0.0f)
    , m_last_read_time(0)
    , m_last_error(SensorStatus::OK)
    , m_initialized(false) {
}

RP2040TempSensor::~RP2040TempSensor() {
    // Nothing to clean up for internal sensor
}

SensorStatus RP2040TempSensor::initialize() {
    // Initialize ADC
    adc_init();
    
    // Enable temperature sensor
    adc_set_temp_sensor_enabled(true);
    
    m_initialized = true;
    m_last_error = SensorStatus::OK;
    
    return SensorStatus::OK;
}

bool RP2040TempSensor::isReady() const {
    return m_initialized;
}

std::string RP2040TempSensor::getName() const {
    return "RP2040 Internal Temperature";
}

SensorStatus RP2040TempSensor::read() {
    if (!m_initialized) {
        m_last_error = SensorStatus::ERROR_NOT_INITIALIZED;
        return m_last_error;
    }
    
    // Select temperature sensor ADC input
    adc_select_input(ADC_TEMP_CHANNEL);
    
    // Read raw ADC value
    uint16_t raw_adc = readInternalTempRaw();
    
    // Convert to temperature
    m_last_temperature = convertRawToTemperature(raw_adc);
    m_last_read_time = to_us_since_boot(get_absolute_time());
    
    m_last_error = SensorStatus::OK;
    return SensorStatus::OK;
}

SensorStatus RP2040TempSensor::getLastError() const {
    return m_last_error;
}

SensorStatus RP2040TempSensor::reset() {
    m_last_temperature = 0.0f;
    m_last_read_time = 0;
    m_last_error = SensorStatus::OK;
    
    if (m_initialized) {
        return initialize(); // Re-initialize
    }
    
    return SensorStatus::OK;
}

float RP2040TempSensor::getTemperatureCelsius() const {
    return m_last_temperature;
}

float RP2040TempSensor::getTemperatureFahrenheit() const {
    return (m_last_temperature * 9.0f / 5.0f) + 32.0f;
}

uint16_t RP2040TempSensor::readInternalTempRaw() {
    // Take multiple readings and average for better accuracy
    constexpr int num_samples = 8;
    uint32_t sum = 0;
    
    for (int i = 0; i < num_samples; i++) {
        sum += adc_read();
        sleep_us(10); // Small delay between readings
    }
    
    return static_cast<uint16_t>(sum / num_samples);
}

float RP2040TempSensor::convertRawToTemperature(uint16_t raw_adc) {
    // Convert ADC reading to voltage
    float voltage = raw_adc * CONVERSION_FACTOR;
    
    // Convert voltage to temperature using RP2040 calibration formula from official Pico SDK
    // The formula is: T = 27 - (ADC_voltage - 0.706) / 0.001721
    float temperature = TEMP_OFFSET - (voltage - TEMP_BIAS) / TEMP_SLOPE;
    
    // Apply user-configurable calibration offset to correct for chip variations
    temperature += Config::TEMPERATURE_CALIBRATION_OFFSET_C;
    
    // Debug logging to help diagnose temperature calibration issues
    // Uncomment these lines for debugging:
    // printf("Temperature sensor debug: raw_adc=%u, voltage=%.4fV, temp_raw=%.2f°C, temp_calibrated=%.2f°C\n", 
    //        raw_adc, voltage, temperature - Config::TEMPERATURE_CALIBRATION_OFFSET_C, temperature);
    
    return temperature;
}
