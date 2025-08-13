/**
 * @file payload_config.hpp
 * @brief MIOTY payload structure configuration
 * 
 * This file defines the structure and content of data payloads sent via MIOTY.
 * It provides a flexible way to define what information goes into which byte
 * of the transmission payload.
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <cstring>

namespace PayloadConfig {
    // Payload structure version for compatibility tracking
    constexpr uint8_t PAYLOAD_VERSION = 1;
    
    // Maximum payload size for MIOTY transmission
    constexpr size_t MAX_PAYLOAD_SIZE = 245;
    
    // Trigger types that can cause an uplink transmission
    enum class TriggerType : uint8_t {
        TIMER = 0x01,           // Scheduled timer-based transmission
        BUTTON = 0x02,          // Button press interrupt
        SENSOR_THRESHOLD = 0x03, // Sensor threshold exceeded
        BATTERY_LOW = 0x04,     // Low battery warning
        ERROR_CONDITION = 0x05,  // Error or fault condition
        MANUAL = 0x06,          // Manual trigger via command
        RFU_1 = 0x07,          // Reserved for future use
        RFU_2 = 0x08           // Reserved for future use
    };
    
    // Sensor types that can be included in payload
    enum class SensorType : uint8_t {
        INTERNAL_TEMPERATURE = 0x01,  // RP2040 internal temperature sensor
        EXTERNAL_TEMPERATURE = 0x02,  // External temperature sensor
        HUMIDITY = 0x03,             // Humidity sensor
        PRESSURE = 0x04,             // Barometric pressure sensor
        BATTERY_VOLTAGE = 0x05,      // Battery voltage measurement
        LIGHT_INTENSITY = 0x06,      // Light/LUX sensor
        ACCELERATION = 0x07,         // Accelerometer data
        GPIO_STATE = 0x08,           // Digital GPIO states
        COUNTER = 0x09,              // Counter/pulse counter
        RFU_SENSOR = 0x0A            // Reserved for future sensor types
    };
    
    // Header structure (fixed position at start of payload)
    struct PayloadHeader {
        uint8_t version;           // Byte 0: Payload structure version
        uint8_t firmware_major;    // Byte 1: Firmware version major
        uint8_t firmware_minor;    // Byte 2: Firmware version minor
        uint8_t hardware_version;  // Byte 3: Hardware version
        uint8_t tx_power_dbm;     // Byte 4: Current TX power setting
        uint8_t trigger_type;     // Byte 5: What triggered this transmission
        uint8_t rfu1;             // Byte 6: Reserved for future use
        uint8_t rfu2;             // Byte 7: Reserved for future use
        
        static constexpr size_t SIZE = 8; // Header is always 8 bytes
    };
    
    // Configuration for current payload structure
    namespace CurrentConfig {
        // Firmware version for payload header (1-byte each)
        constexpr uint8_t FW_MAJOR = 1;
        constexpr uint8_t FW_MINOR = 0;
        
        // Hardware version for payload header (1-byte)
        constexpr uint8_t HW_VERSION = 1;
        
        // Default trigger type
        constexpr TriggerType DEFAULT_TRIGGER = TriggerType::TIMER;
        
        // Sensor configuration - defines which sensors to include and their format
        struct SensorConfig {
            SensorType type;
            uint8_t data_format;    // 0=uint8, 1=int16 (big endian), 2=uint16 (big endian), 3=int32 (big endian)
            uint16_t multiplier;    // For fixed-point representation
            uint8_t data_length;    // Bytes needed for this sensor
        };
        
        // Current sensor configuration array
        constexpr SensorConfig SENSOR_CONFIGS[] = {
            // Internal temperature: int16 with 100x multiplier for 0.01°C precision (big endian)
            {SensorType::INTERNAL_TEMPERATURE, 1, 100, 2},
            
            // Add more sensors here as needed:
            // {SensorType::HUMIDITY, 2, 100, 2},           // uint16, 100x multiplier, 0.01% precision
            // {SensorType::BATTERY_VOLTAGE, 2, 1000, 2},   // uint16, 1000x multiplier, 0.001V precision
        };
        
        constexpr size_t SENSOR_COUNT = sizeof(SENSOR_CONFIGS) / sizeof(SENSOR_CONFIGS[0]);
    }
    
    // Helper functions for payload assembly and parsing
    class PayloadBuilder {
    public:
        PayloadBuilder();
        ~PayloadBuilder() = default;
        
        /**
         * @brief Reset the payload buffer for a new message
         */
        void reset();
        
        /**
         * @brief Set the trigger type for this transmission
         * @param trigger The trigger that caused this transmission
         */
        void setTrigger(TriggerType trigger);
        
        /**
         * @brief Add sensor data to the payload
         * @param sensor_type Type of sensor
         * @param value Sensor value (will be converted based on config)
         * @return true if successfully added, false if payload full
         */
        bool addSensorData(SensorType sensor_type, float value);
        
        /**
         * @brief Add raw sensor data to the payload
         * @param sensor_type Type of sensor
         * @param data Raw data bytes
         * @param length Length of data
         * @return true if successfully added, false if payload full
         */
        bool addRawSensorData(SensorType sensor_type, const uint8_t* data, size_t length);
        
        /**
         * @brief Finalize the payload and get the complete data buffer
         * @param tx_power_dbm Current TX power setting to include in header
         * @param length_out Output parameter for payload length
         * @return Pointer to payload data buffer
         */
        const uint8_t* getPayload(uint8_t tx_power_dbm, size_t* length_out) const;
        
        /**
         * @brief Get current payload size
         * @return Current size of payload in bytes
         */
        size_t getPayloadSize() const { return m_payload_size; }
        
        /**
         * @brief Check if payload has space for more data
         * @param bytes_needed Number of bytes needed
         * @return true if space available
         */
        bool hasSpace(size_t bytes_needed) const;
        
    private:
        uint8_t m_payload_buffer[MAX_PAYLOAD_SIZE];
        size_t m_payload_size;
        TriggerType m_trigger_type;
        
        /**
         * @brief Write payload header to buffer
         * @param tx_power_dbm Current TX power setting
         */
        void writeHeader(uint8_t tx_power_dbm);
        
        /**
         * @brief Find sensor config for given sensor type
         * @param sensor_type Type of sensor to find
         * @return Pointer to config or nullptr if not found
         */
        const CurrentConfig::SensorConfig* findSensorConfig(SensorType sensor_type) const;
        
        /**
         * @brief Convert float value to fixed-point representation
         * @param value Float value to convert
         * @param multiplier Fixed-point multiplier
         * @param data_format Data format (0=uint8, 1=int16, 2=uint16, 3=int32)
         * @param output Output buffer for converted data
         * @return Number of bytes written to output
         */
        size_t convertValueToBytes(float value, uint16_t multiplier, uint8_t data_format, uint8_t* output) const;
    };
    
    // Utility functions
    namespace Utils {
        /**
         * @brief Convert trigger type enum to string for logging
         * @param trigger Trigger type
         * @return String representation
         */
        const char* triggerTypeToString(TriggerType trigger);
        
        /**
         * @brief Convert sensor type enum to string for logging
         * @param sensor Sensor type
         * @return String representation
         */
        const char* sensorTypeToString(SensorType sensor);
        
        /**
         * @brief Calculate expected payload size for current configuration
         * @return Expected payload size in bytes
         */
        size_t calculateExpectedPayloadSize();
    }
}
