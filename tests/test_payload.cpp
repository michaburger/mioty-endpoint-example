/**
 * @file test_payload.cpp
 * @brief Simple payload configuration test
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#include "../src/config/payload_config.hpp"
#include <cstdio>
#include <cstring>

void print_hex(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main() {
    printf("=== MIOTY Payload Configuration Test ===\n\n");
    
    // Test payload builder
    PayloadConfig::PayloadBuilder builder;
    
    // Test 1: Basic temperature payload
    printf("Test 1: Basic temperature payload\n");
    builder.reset();
    builder.setTrigger(PayloadConfig::TriggerType::TIMER);
    
    // Add temperature sensor reading: 23.45°C
    if (builder.addSensorData(PayloadConfig::SensorType::INTERNAL_TEMPERATURE, 23.45f)) {
        printf("✓ Temperature sensor added successfully\n");
    } else {
        printf("✗ Failed to add temperature sensor\n");
        return 1;
    }
    
    // Get payload
    size_t payload_length;
    const uint8_t* payload_data = builder.getPayload(20, &payload_length); // 20 dBm TX power
    
    printf("Payload length: %u bytes\n", (unsigned)payload_length);
    printf("Payload hex: ");
    print_hex(payload_data, payload_length);
    
    // Verify payload structure
    printf("Header analysis:\n");
    printf("  Version: %u\n", payload_data[0]);
    printf("  FW Version: %u.%u\n", payload_data[1], payload_data[2]);
    printf("  HW Version: %u\n", payload_data[3]);
    printf("  TX Power: %u dBm\n", payload_data[4]);
    printf("  Trigger: %s\n", PayloadConfig::Utils::triggerTypeToString(
        static_cast<PayloadConfig::TriggerType>(payload_data[5])));
    printf("  RFU1: %u\n", payload_data[6]);
    printf("  RFU2: %u\n", payload_data[7]);
    
    if (payload_length >= 10) { // Header + 2 bytes temperature data
        printf("Sensor data analysis:\n");
        printf("  Temperature data format: int16, multiplier=100 (defined by FW version)\n");
        
        // Temperature data starts at byte 8 (after 8-byte header)
        int16_t temp_raw = payload_data[8] | (payload_data[9] << 8);
        float temp_celsius = temp_raw / 100.0f;
        printf("  Temperature: %d raw → %.2f°C\n", temp_raw, temp_celsius);
    }
    
    printf("\n");
    
    // Test 2: Expected payload size calculation
    printf("Test 2: Payload size calculation\n");
    size_t expected_size = PayloadConfig::Utils::calculateExpectedPayloadSize();
    printf("Expected payload size: %u bytes\n", (unsigned)expected_size);
    printf("Actual payload size: %u bytes\n", (unsigned)payload_length);
    
    if (expected_size == payload_length) {
        printf("✓ Payload size matches expectation\n");
    } else {
        printf("✗ Payload size mismatch!\n");
        return 1;
    }
    
    printf("\n");
    
    // Test 3: Multiple trigger types
    printf("Test 3: Different trigger types\n");
    
    PayloadConfig::TriggerType triggers[] = {
        PayloadConfig::TriggerType::TIMER,
        PayloadConfig::TriggerType::BUTTON,
        PayloadConfig::TriggerType::SENSOR_THRESHOLD,
        PayloadConfig::TriggerType::BATTERY_LOW
    };
    
    for (size_t i = 0; i < sizeof(triggers)/sizeof(triggers[0]); i++) {
        builder.reset();
        builder.setTrigger(triggers[i]);
        builder.addSensorData(PayloadConfig::SensorType::INTERNAL_TEMPERATURE, 20.0f + i);
        
        const uint8_t* test_payload = builder.getPayload(15, &payload_length);
        printf("  %s trigger: ", PayloadConfig::Utils::triggerTypeToString(triggers[i]));
        print_hex(test_payload, 8); // Just show header
    }
    
    printf("\n=== All tests completed successfully! ===\n");
    
    return 0;
}
