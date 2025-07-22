/**
 * @file payload_config.cpp
 * @brief MIOTY payload structure configuration implementation
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#include "payload_config.hpp"
#include <cstring>
#include <algorithm>

namespace PayloadConfig {

PayloadBuilder::PayloadBuilder() 
    : m_payload_size(0)
    , m_trigger_type(CurrentConfig::DEFAULT_TRIGGER)
{
    memset(m_payload_buffer, 0, sizeof(m_payload_buffer));
}

void PayloadBuilder::reset() {
    m_payload_size = 0;
    m_trigger_type = CurrentConfig::DEFAULT_TRIGGER;
    memset(m_payload_buffer, 0, sizeof(m_payload_buffer));
}

void PayloadBuilder::setTrigger(TriggerType trigger) {
    m_trigger_type = trigger;
}

bool PayloadBuilder::addSensorData(SensorType sensor_type, float value) {
    const CurrentConfig::SensorConfig* config = findSensorConfig(sensor_type);
    if (!config) {
        return false; // Sensor type not configured
    }
    
    // Check if we have space (accounting for header that will be written later)
    if (!hasSpace(PayloadHeader::SIZE + config->data_length)) {
        return false;
    }
    
    // Convert value to bytes based on configuration
    uint8_t converted_data[4]; // Max 4 bytes for int32
    size_t bytes_written = convertValueToBytes(value, config->multiplier, config->data_format, converted_data);
    
    if (bytes_written != config->data_length) {
        return false; // Conversion error
    }
    
    // Skip header space if this is the first sensor (header will be written in getPayload())
    size_t write_offset = (m_payload_size == 0) ? PayloadHeader::SIZE : m_payload_size;
    
    // Write sensor data directly (no sensor entry header needed)
    memcpy(&m_payload_buffer[write_offset], converted_data, config->data_length);
    m_payload_size = write_offset + config->data_length;
    
    return true;
}

bool PayloadBuilder::addRawSensorData(SensorType sensor_type, const uint8_t* data, size_t length) {
    const CurrentConfig::SensorConfig* config = findSensorConfig(sensor_type);
    if (!config || length != config->data_length) {
        return false; // Sensor not configured or length mismatch
    }
    
    // Check if we have space (accounting for header that will be written later)
    if (!hasSpace(PayloadHeader::SIZE + length)) {
        return false;
    }
    
    // Skip header space if this is the first sensor
    size_t write_offset = (m_payload_size == 0) ? PayloadHeader::SIZE : m_payload_size;
    
    // Write sensor data directly (no sensor entry header needed)
    memcpy(&m_payload_buffer[write_offset], data, length);
    m_payload_size = write_offset + length;
    
    return true;
}

const uint8_t* PayloadBuilder::getPayload(uint8_t tx_power_dbm, size_t* length_out) const {
    // Write header at the beginning (this is safe because we reserved space)
    const_cast<PayloadBuilder*>(this)->writeHeader(tx_power_dbm);
    
    if (length_out) {
        *length_out = m_payload_size;
    }
    
    return m_payload_buffer;
}

bool PayloadBuilder::hasSpace(size_t bytes_needed) const {
    return (m_payload_size + bytes_needed) <= MAX_PAYLOAD_SIZE;
}

void PayloadBuilder::writeHeader(uint8_t tx_power_dbm) {
    PayloadHeader header;
    header.version = PAYLOAD_VERSION;
    header.firmware_major = CurrentConfig::FW_MAJOR;
    header.firmware_minor = CurrentConfig::FW_MINOR;
    header.hardware_version = CurrentConfig::HW_VERSION;
    header.tx_power_dbm = tx_power_dbm;
    header.trigger_type = static_cast<uint8_t>(m_trigger_type);
    header.rfu1 = 0;  // Reserved for future use
    header.rfu2 = 0;  // Reserved for future use
    
    // Write header to beginning of buffer
    memcpy(m_payload_buffer, &header, PayloadHeader::SIZE);
}

const CurrentConfig::SensorConfig* PayloadBuilder::findSensorConfig(SensorType sensor_type) const {
    for (size_t i = 0; i < CurrentConfig::SENSOR_COUNT; i++) {
        if (CurrentConfig::SENSOR_CONFIGS[i].type == sensor_type) {
            return &CurrentConfig::SENSOR_CONFIGS[i];
        }
    }
    return nullptr;
}

size_t PayloadBuilder::convertValueToBytes(float value, uint16_t multiplier, uint8_t data_format, uint8_t* output) const {
    // Apply multiplier for fixed-point representation
    int32_t fixed_point_value = static_cast<int32_t>(value * multiplier);
    
    switch (data_format) {
        case 0: // uint8
            if (fixed_point_value < 0 || fixed_point_value > 255) {
                return 0; // Value out of range
            }
            output[0] = static_cast<uint8_t>(fixed_point_value);
            return 1;
            
        case 1: // int16 (little endian)
            if (fixed_point_value < -32768 || fixed_point_value > 32767) {
                return 0; // Value out of range
            }
            output[0] = fixed_point_value & 0xFF;
            output[1] = (fixed_point_value >> 8) & 0xFF;
            return 2;
            
        case 2: // uint16 (little endian)
            if (fixed_point_value < 0 || fixed_point_value > 65535) {
                return 0; // Value out of range
            }
            output[0] = fixed_point_value & 0xFF;
            output[1] = (fixed_point_value >> 8) & 0xFF;
            return 2;
            
        case 3: // int32 (little endian)
            output[0] = fixed_point_value & 0xFF;
            output[1] = (fixed_point_value >> 8) & 0xFF;
            output[2] = (fixed_point_value >> 16) & 0xFF;
            output[3] = (fixed_point_value >> 24) & 0xFF;
            return 4;
            
        default:
            return 0; // Unknown format
    }
}

namespace Utils {

const char* triggerTypeToString(PayloadConfig::TriggerType trigger) {
    switch (trigger) {
        case PayloadConfig::TriggerType::TIMER: return "TIMER";
        case PayloadConfig::TriggerType::BUTTON: return "BUTTON";
        case PayloadConfig::TriggerType::SENSOR_THRESHOLD: return "SENSOR_THRESHOLD";
        case PayloadConfig::TriggerType::BATTERY_LOW: return "BATTERY_LOW";
        case PayloadConfig::TriggerType::ERROR_CONDITION: return "ERROR_CONDITION";
        case PayloadConfig::TriggerType::MANUAL: return "MANUAL";
        case PayloadConfig::TriggerType::RFU_1: return "RFU_1";
        case PayloadConfig::TriggerType::RFU_2: return "RFU_2";
        default: return "UNKNOWN";
    }
}

const char* sensorTypeToString(PayloadConfig::SensorType sensor) {
    switch (sensor) {
        case PayloadConfig::SensorType::INTERNAL_TEMPERATURE: return "INTERNAL_TEMPERATURE";
        case PayloadConfig::SensorType::EXTERNAL_TEMPERATURE: return "EXTERNAL_TEMPERATURE";
        case PayloadConfig::SensorType::HUMIDITY: return "HUMIDITY";
        case PayloadConfig::SensorType::PRESSURE: return "PRESSURE";
        case PayloadConfig::SensorType::BATTERY_VOLTAGE: return "BATTERY_VOLTAGE";
        case PayloadConfig::SensorType::LIGHT_INTENSITY: return "LIGHT_INTENSITY";
        case PayloadConfig::SensorType::ACCELERATION: return "ACCELERATION";
        case PayloadConfig::SensorType::GPIO_STATE: return "GPIO_STATE";
        case PayloadConfig::SensorType::COUNTER: return "COUNTER";
        case PayloadConfig::SensorType::RFU_SENSOR: return "RFU_SENSOR";
        default: return "UNKNOWN";
    }
}

size_t calculateExpectedPayloadSize() {
    size_t size = PayloadConfig::PayloadHeader::SIZE;
    
    // Add size for each configured sensor (just the data, no sensor entry headers)
    for (size_t i = 0; i < PayloadConfig::CurrentConfig::SENSOR_COUNT; i++) {
        size += PayloadConfig::CurrentConfig::SENSOR_CONFIGS[i].data_length;
    }
    
    return size;
}

} // namespace Utils
} // namespace PayloadConfig
