/* -----------------------------------------------------------------------------

Software License for the Fraunhofer TS-UNB-Lib

(c) Copyright  2019 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. All rights reserved.


1. INTRODUCTION

The Fraunhofer Telegram Splitting - Ultra Narrowband Library ("TS-UNB-Lib") is software
that implements only the uplink of the ETSI TS 103 357 TS-UNB standard ("MIOTY") for wireless 
data transmission in the field of IoT. Patent licenses for any patent claim regarding the 
ETSI TS 103 357 TS-UNB standard implementation (including those of Fraunhofer) may be 
obtained through Sisvel International S.A. 
(https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms)
or through the respective patent owners individually. The purpose of this TS-UNB-Lib is 
academic and non-commercial use. Therefore, Fraunhofer does not offer any support for the 
TS-UNB-Lib. Furthermore, the TS-UNB-Lib is NOT identical and on the same quality level as 
the commercially-licensed MIOTY software also available from Fraunhofer. Users are encouraged
to check the Fraunhofer website for additional applications information and documentation.


2. COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification, are 
permitted without payment of copyright license fees provided that you satisfy the following 
conditions: You must retain the complete text of this software license in redistributions
of the TS-UNB-Lib software or your modifications thereto in source code form. You must retain 
the complete text of this software license in the documentation and/or other materials provided
with redistributions of the TS-UNB-Lib software or your modifications thereto in binary form.
You must make available free of charge copies of the complete source code of the TS-UNB-Lib 
software and your modifications thereto to recipients of copies in binary form. The name of 
Fraunhofer may not be used to endorse or promote products derived from this software without
prior written permission. You may not charge copyright license fees for anyone to use, copy or
distribute the TS-UNB-Lib software or your modifications thereto. Your modified versions of the
TS-UNB-Lib software must carry prominent notices stating that you changed the software and the
date of any change. For modified versions of the TS-UNB-Lib software, the term 
"Fraunhofer TS-UNB-Lib" must be replaced by the term
"Third-Party Modified Version of the Fraunhofer TS-UNB-Lib."


3. NO PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without limitation the patents 
of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE. Fraunhofer provides no warranty of patent 
non-infringement with respect to this software. You may use this TS-UNB-Lib software or modifications
thereto only for purposes that are authorized by appropriate patent licenses.


4. DISCLAIMER

This TS-UNB-Lib software is provided by Fraunhofer on behalf of the copyright holders and contributors
"AS IS" and WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, including but not limited to the implied warranties
of merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE for any direct, indirect, incidental, special, exemplary, or consequential damages,
including but not limited to procurement of substitute goods or services; loss of use, data, or profits,
or business interruption, however caused and on any theory of liability, whether in contract, strict
liability, or tort (including negligence), arising in any way out of the use of this software, even if
advised of the possibility of such damage.


5. CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Division Communication Systems
Am Wolfsmantel 33
91058 Erlangen, Germany
ks-contracts@iis.fraunhofer.de

This file is part of a Third-Party Modified Version of the Fraunhofer TS-UNB-Lib.
Modifications by mioty Alliance e.V. (2025)

----------------------------------------------------------------------------- */

/**
 * @file application.cpp
 * @brief Main application logic implementation with TS-UNB integration
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#include "application.hpp"
#include "../config/app_config.hpp"
#include "../../lib/utils/logger.hpp"
#include "hardware/watchdog.h"
#include "pico/time.h"
#include "pico/unique_id.h"
#include <cstdio>
#include <cstring>

Application::Application()
    : m_board_config()
    , m_ts_unb_driver()
    , m_last_sensor_reading_time(0)
    , m_last_transmission_time(0)
    , m_sensor_data({0.0f})
    , m_packet_counter(0)
    , m_device_eui64{0}
    , m_device_short_addr{0}
    , m_is_running(false)
{
}

Application::~Application() {
    if (m_is_running) {
        stop();
    }
}

bool Application::initialize() {
    // Give time for terminal connection before logging important startup info
    sleep_ms(2000);
    
    // Enable debug logging if configured
    if (Config::ENABLE_DEBUG_OUTPUT) {
        Logger::setLogLevel(LogLevel::DEBUG);
        Logger::debug("Debug logging enabled");
    }
    
    Logger::info("Initializing %s v%s", Config::APP_NAME, Config::APP_VERSION);
    
    // Initialize board configuration
    if (!m_board_config.initialize()) {
        Logger::error("Board configuration initialization failed");
        return false;
    }
    
    // Initialize persistent storage for frame counter
    if (!m_frame_counter_storage.initialize()) {
        Logger::error("Persistent frame counter storage initialization failed");
        return false;
    }
    
    // Print unique board ID
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    Logger::info("Board ID: %02x%02x%02x%02x%02x%02x%02x%02x", 
                 board_id.id[0], board_id.id[1], board_id.id[2], board_id.id[3],
                 board_id.id[4], board_id.id[5], board_id.id[6], board_id.id[7]);
    
    // Initialize communication systems
    if (Config::ENABLE_MIOTY && !initializeCommunication()) {
        Logger::error("Communication initialization failed");
        return false;
    }
    
    // Initialize sensors (placeholder)
    if (!initializeSensors()) {
        Logger::error("Sensor initialization failed");
        return false;
    }
    
    // Initialize power bank keep-alive if enabled
    if (Config::POWER_FROM_POWERBANK) {
        uint8_t led_gpio = Config::PowerBankKeepAlive::ENABLE_LOAD_LED_INDICATOR ? 
                          Board::LED_PIN : UINT8_MAX;
        
        if (!m_powerbank_keepalive.initialize(
            Board::GPIO::POWERBANK_LOAD_PIN,
            led_gpio,
            Config::PowerBankKeepAlive::PULSE_INTERVAL_MS,
            Config::PowerBankKeepAlive::PULSE_DURATION_MS,
            Config::PowerBankKeepAlive::USE_EXTERNAL_RESISTOR)) {
            Logger::error("Power bank keep-alive initialization failed");
            return false;
        }
        
        Logger::info("Power bank keep-alive enabled - GPIO: %d, Interval: %dms, External resistor: %s", 
                     Board::GPIO::POWERBANK_LOAD_PIN,
                     Config::PowerBankKeepAlive::PULSE_INTERVAL_MS,
                     Config::PowerBankKeepAlive::USE_EXTERNAL_RESISTOR ? "Yes" : "No");
    }
    
    // Initialize and log payload configuration
    Logger::info("Payload system initialized - Expected payload size: %u bytes", 
                 (unsigned)PayloadConfig::Utils::calculateExpectedPayloadSize());
    Logger::info("Configured sensors: %u", (unsigned)PayloadConfig::CurrentConfig::SENSOR_COUNT);
    
    // Enable watchdog if configured
    if (Config::WATCHDOG_TIMEOUT_MS > 0) {
        watchdog_enable(Config::WATCHDOG_TIMEOUT_MS, true);
        Logger::info("Watchdog enabled with %u ms timeout", Config::WATCHDOG_TIMEOUT_MS);
    }
    
    Logger::info("Application initialization completed successfully");
    return true;
}

void Application::run() {
    Logger::info("Starting main application loop");
    m_is_running = true;
    
    uint32_t loop_counter = 0;
    
    while (m_is_running) {
        absolute_time_t loop_start = get_absolute_time();
        
        // Update watchdog
        if (Config::WATCHDOG_TIMEOUT_MS > 0) {
            watchdog_update();
        }
        
        // Handle sensor readings
        if (shouldReadSensors()) {
            readSensors();
            m_last_sensor_reading_time = to_ms_since_boot(get_absolute_time());
        }
        
        // Handle data transmission
        if (shouldTransmitData()) {
            // Set transmission time at start to ensure exact intervals
            m_last_transmission_time = to_ms_since_boot(get_absolute_time());
            transmitData();
        }
        
        // Update board status (LED, etc.)
        updateBoardStatus();
        
        // Update power bank keep-alive if enabled
        if (Config::POWER_FROM_POWERBANK) {
            m_powerbank_keepalive.update();
        }
        
        // Log periodic status
        if (++loop_counter % 60 == 0) { // Every 60 seconds with 1s loop
            Logger::info("Application running - Loop #%u, Packets sent: %u", loop_counter, m_packet_counter);
            logDeviceIdentity();
        }
        
        // Calculate remaining time for this loop iteration
        absolute_time_t loop_end = get_absolute_time();
        int64_t elapsed_us = absolute_time_diff_us(loop_start, loop_end);
        int64_t delay_us = (Config::MAIN_LOOP_DELAY_MS * 1000) - elapsed_us;
        
        if (delay_us > 0) {
            sleep_us(delay_us);
        } else {
            Logger::warning("Main loop overrun by %lld us", -delay_us);
        }
    }
    
    Logger::info("Main application loop ended");
}

void Application::stop() {
    Logger::info("Stopping application");
    m_is_running = false;
}

bool Application::initializeCommunication() {
    Logger::info("Initializing TS-UNB communication (Third-Party Modified Version of the Fraunhofer TS-UNB-Lib)");
    
    // Get unique board ID for device identity
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    
    // Create node configuration from centralized app config
    TSUNBDriver::NodeConfig config = createNodeConfigFromAppConfig(board_id.id);
    
    // Helper functions for logging enum values
    const char* regionStr = (config.region == TSUNBDriver::Region::EU0) ? "EU0" :
                           (config.region == TSUNBDriver::Region::EU1) ? "EU1" :
                           (config.region == TSUNBDriver::Region::EU2) ? "EU2" :
                           (config.region == TSUNBDriver::Region::US0) ? "US0" : "Unknown";
    
    const char* chipStr = (config.chip_type == TSUNBDriver::ChipType::RFM69W) ? "RFM69W" :
                         (config.chip_type == TSUNBDriver::ChipType::RFM69HW) ? "RFM69HW" : "Unknown";
    
    Logger::info("TS-UNB Config - Region: %s, Chip: %s, Power: %d dBm", 
                 regionStr, chipStr, config.tx_power_dbm);
    
    // Store device identity for later logging purposes
    memcpy(m_device_eui64, config.eui64, 8);
    memcpy(m_device_short_addr, config.short_addr, 2);
    
    // Enhanced device identity logging for easy identification
    Logger::info("=== DEVICE IDENTITY ===");
    Logger::info("Device EUI64: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
                 config.eui64[0], config.eui64[1], config.eui64[2], config.eui64[3],
                 config.eui64[4], config.eui64[5], config.eui64[6], config.eui64[7]);
    Logger::info("Short Address: 0x%02X%02X", config.short_addr[0], config.short_addr[1]);
    if (Config::ENABLE_NETWORK_KEY_DEBUG) {
        Logger::info("Network Key: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
                     config.network_key[0], config.network_key[1], config.network_key[2], config.network_key[3],
                     config.network_key[4], config.network_key[5], config.network_key[6], config.network_key[7],
                     config.network_key[8], config.network_key[9], config.network_key[10], config.network_key[11],
                     config.network_key[12], config.network_key[13], config.network_key[14], config.network_key[15]);
    }
    Logger::info("=======================");
    
    // Initialize the TS-UNB driver
    TSUNBStatus status = m_ts_unb_driver.initialize(config);
    if (status != TSUNBStatus::OK) {
        Logger::error("TS-UNB driver initialization failed with status %d", static_cast<int>(status));
        return false;
    }
    
    Logger::info("TS-UNB communication initialized successfully");
    return true;
}

bool Application::initializeSensors() {
    Logger::info("Initializing sensors");
    
    // Initialize sensor data structure
    m_sensor_data.temperature = 20.0f; // Default value
    
    // Initialize temperature sensor
    if (m_temperature_sensor.initialize() != SensorStatus::OK) {
        Logger::error("Failed to initialize internal temperature sensor");
        return false;
    }
    
    Logger::info("Temperature sensor initialized successfully");
    Logger::info("Sensors initialized");
    return true;
}

bool Application::shouldReadSensors() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    return (current_time - m_last_sensor_reading_time) >= Config::TEMPERATURE_SAMPLE_INTERVAL_MS;
}

bool Application::shouldTransmitData() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    return (current_time - m_last_transmission_time) >= Config::MIOTY_TRANSMISSION_INTERVAL_MS;
}

void Application::readSensors() {
    Logger::debug("=== SENSOR READING ===");
    
    // Read temperature from internal sensor
    if (m_temperature_sensor.read() == SensorStatus::OK) {
        m_sensor_data.temperature = m_temperature_sensor.getTemperatureCelsius();
        Logger::info("Temperature sensor reading: %.2f°C", m_sensor_data.temperature);
    } else {
        Logger::warning("Failed to read temperature sensor, using previous value: %.2f°C", m_sensor_data.temperature);
    }
    
    // Only log temperature since other sensors are not implemented
    Logger::debug("Sensors read - T: %.1f°C", m_sensor_data.temperature);
}

void Application::transmitData() {
    if (!m_ts_unb_driver.isInitialized()) {
        Logger::warning("TS-UNB driver not initialized, skipping transmission");
        return;
    }
    
    if (m_ts_unb_driver.isTransmitting()) {
        Logger::warning("TS-UNB transmission in progress, skipping");
        return;
    }
    
    // Log transmission timing for debugging
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    uint32_t time_since_last = current_time - m_last_transmission_time;
    Logger::debug("Starting transmission - Current: %u ms, Since last: %u ms", current_time, time_since_last);
    
    // Reset payload builder for new transmission
    m_payload_builder.reset();
    m_payload_builder.setTrigger(PayloadConfig::TriggerType::TIMER);
    
    // Add sensor data based on payload configuration
    bool sensor_added = false;
    
    // Add internal temperature sensor data
    if (m_payload_builder.addSensorData(PayloadConfig::SensorType::INTERNAL_TEMPERATURE, m_sensor_data.temperature)) {
        sensor_added = true;
        Logger::debug("Added temperature sensor data: %.2f°C", m_sensor_data.temperature);
    } else {
        Logger::warning("Failed to add temperature sensor data to payload");
    }
    
    // TODO: Add other sensors as they are configured in payload_config.hpp
    
    if (!sensor_added) {
        Logger::error("No sensor data added to payload, skipping transmission");
        return;
    }
    
    // Get the assembled payload
    size_t payload_length;
    const uint8_t* payload_data = m_payload_builder.getPayload(static_cast<uint8_t>(Config::Mioty::TX_POWER_DBM), &payload_length);
    
    if (payload_length == 0) {
        Logger::error("Empty payload generated, skipping transmission");
        return;
    }
    
    // Log payload information
    Logger::info("=== MIOTY TRANSMISSION #%u ===", ++m_packet_counter);
    Logger::info("Payload size: %u bytes", (unsigned)payload_length);
    
    // Log complete payload bytes as hex
    Logger::info("Payload bytes (hex): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
                 payload_data[0], payload_data[1], payload_data[2], payload_data[3], payload_data[4],
                 payload_data[5], payload_data[6], payload_data[7], payload_data[8], payload_data[9]);
    
    // Log payload structure breakdown for debugging
    Logger::debug("Payload structure - Version: %u, FW: %u.%u, HW: %u, TX Power: %u dBm, Trigger: %s, RFU1: %u, RFU2: %u", 
                  payload_data[0], payload_data[1], payload_data[2], payload_data[3], 
                  payload_data[4], PayloadConfig::Utils::triggerTypeToString(static_cast<PayloadConfig::TriggerType>(payload_data[5])), 
                  payload_data[6], payload_data[7]);
    
    // Log sensor data bytes (last 2 bytes are sensor data)
    Logger::debug("Sensor data bytes: [8]=0x%02X [9]=0x%02X (temperature: %.2f°C)", 
                  payload_data[8], payload_data[9], m_sensor_data.temperature);
    
    // Send the binary data via TS-UNB
    TSUNBStatus status = m_ts_unb_driver.sendData(payload_data, payload_length);
    
    if (status == TSUNBStatus::OK) {
        Logger::info("✓ MIOTY transmission successful (packet #%u)", m_packet_counter);
        
        // Save the updated frame counter to persistent storage
        uint32_t current_frame_counter = m_ts_unb_driver.getFrameCounter();
        m_frame_counter_storage.writeFrameCounter(current_frame_counter);
        Logger::debug("Frame counter saved to persistent storage: %u", current_frame_counter);
        
        logDeviceIdentity();
        Logger::info("================================");
    } else {
        Logger::error("✗ MIOTY transmission FAILED with status %d (packet #%u)", static_cast<int>(status), m_packet_counter);
        logDeviceIdentity();
        Logger::info("================================");
        // Don't increment packet counter on failure
        --m_packet_counter;
    }
}

void Application::updateBoardStatus() {
    // Blink status LED to show the system is alive
    static uint32_t last_blink_time = 0;
    static bool led_state = false;
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if ((current_time - last_blink_time) >= Config::LED_BLINK_DELAY_MS) {
        led_state = !led_state;
        m_board_config.setStatusLED(led_state);
        last_blink_time = current_time;
    }
}

TSUNBDriver::NodeConfig Application::createNodeConfigFromAppConfig(const uint8_t board_id[8]) {
    TSUNBDriver::NodeConfig config;
    
    // Set configuration directly from enum values (no string parsing needed!)
    config.region = Config::Mioty::REGION;
    config.chip_type = Config::Mioty::CHIP_TYPE;
    config.tx_power_dbm = Config::Mioty::TX_POWER_DBM;
    
    // Copy network key from configuration
    memcpy(config.network_key, Config::Mioty::NETWORK_KEY, 16);
    
    // Configure device identity based on settings
    if (Config::Mioty::USE_BOARD_ID_FOR_EUI64) {
        // Use board ID for unique EUI64
        memcpy(config.eui64, board_id, 8);
    } else {
        // Use static EUI64 from configuration
        memcpy(config.eui64, Config::Mioty::STATIC_EUI64, 8);
    }
    
    if (Config::Mioty::USE_BOARD_ID_FOR_SHORT_ADDR) {
        // Generate short address from last 2 bytes of board ID
        config.short_addr[0] = board_id[6];
        config.short_addr[1] = board_id[7];
    } else {
        // Use static short address from configuration
        memcpy(config.short_addr, Config::Mioty::STATIC_SHORT_ADDR, 2);
    }
    
    // Set initial extended packet counter from persistent storage
    config.ext_pkg_cnt = m_frame_counter_storage.readFrameCounter();
    Logger::info("Loaded frame counter from persistent storage: %u", config.ext_pkg_cnt);
    
    return config;
}

void Application::logDeviceIdentity() const {
    Logger::info("Device EUI64: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X, Short Addr: 0x%02X%02X", 
                 m_device_eui64[0], m_device_eui64[1], m_device_eui64[2], m_device_eui64[3],
                 m_device_eui64[4], m_device_eui64[5], m_device_eui64[6], m_device_eui64[7],
                 m_device_short_addr[0], m_device_short_addr[1]);
    if (Config::ENABLE_NETWORK_KEY_DEBUG) {
        Logger::info("Network Key: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
                     Config::Mioty::NETWORK_KEY[0], Config::Mioty::NETWORK_KEY[1], Config::Mioty::NETWORK_KEY[2], Config::Mioty::NETWORK_KEY[3],
                     Config::Mioty::NETWORK_KEY[4], Config::Mioty::NETWORK_KEY[5], Config::Mioty::NETWORK_KEY[6], Config::Mioty::NETWORK_KEY[7],
                     Config::Mioty::NETWORK_KEY[8], Config::Mioty::NETWORK_KEY[9], Config::Mioty::NETWORK_KEY[10], Config::Mioty::NETWORK_KEY[11],
                     Config::Mioty::NETWORK_KEY[12], Config::Mioty::NETWORK_KEY[13], Config::Mioty::NETWORK_KEY[14], Config::Mioty::NETWORK_KEY[15]);
    }
}
