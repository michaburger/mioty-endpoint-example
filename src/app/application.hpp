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
 * @file application.hpp
 * @brief Main application class header with TS-UNB integration
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "pico/stdlib.h"
#include "../config/app_config.hpp"
#include "../config/board_config.hpp"
#include "../config/payload_config.hpp"
#include "../../drivers/mioty/ts_unb_driver.hpp"
#include "../../drivers/sensors/temperature/rp2040_temp_sensor.hpp"
#include "../../lib/utils/logger.hpp"
#include "../../lib/utils/powerbank_keepalive.hpp"
#include "../../lib/utils/persistent_storage.hpp"

/**
 * @brief Sensor data structure
 */
struct SensorData {
    float temperature;      // Temperature in Celsius
};

/**
 * @brief Main application class that orchestrates all components
 */
class Application {
public:
    Application();
    ~Application();
    
    /**
     * @brief Initialize all application components
     * @return true if initialization successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Run the main application loop
     */
    void run();
    
    /**
     * @brief Stop the application gracefully
     */
    void stop();

private:
    // Core components
    BoardConfig m_board_config;
    TSUNBDriver m_ts_unb_driver;
    RP2040TempSensor m_temperature_sensor;
    PayloadConfig::PayloadBuilder m_payload_builder;
    PowerBankKeepAlive::KeepAliveManager m_powerbank_keepalive;
    PersistentStorage::FrameCounterStorage m_frame_counter_storage;
    
    // Timing
    uint32_t m_last_sensor_reading_time;
    uint32_t m_last_transmission_time;
    
    // Data
    SensorData m_sensor_data;
    uint32_t m_packet_counter;
    
    // Device identity (stored for logging purposes)
    uint8_t m_device_eui64[8];
    uint8_t m_device_short_addr[2];
    
    // State
    bool m_is_running;
    
    /**
     * @brief Initialize TS-UNB communication
     * @return true if successful
     */
    bool initializeCommunication();
    
    /**
     * @brief Initialize sensors
     * @return true if successful
     */
    bool initializeSensors();
    
    /**
     * @brief Check if it's time to read sensors
     * @return true if sensors should be read
     */
    bool shouldReadSensors();
    
    /**
     * @brief Check if it's time to transmit data
     * @return true if data should be transmitted
     */
    bool shouldTransmitData();
    
    /**
     * @brief Read all sensors
     */
    void readSensors();
    
    /**
     * @brief Transmit data via TS-UNB
     */
    void transmitData();
    
    /**
     * @brief Update board status (LED, etc.)
     */
    void updateBoardStatus();
    
    /**
     * @brief Perform pre-transmission LED blinking sequence
     */
    void performTransmissionBlink();
    
    /**
     * @brief Create NodeConfig from centralized app configuration
     * @param board_id Unique 8-byte board identifier
     * @return Configured NodeConfig for TS-UNB driver
     */
    TSUNBDriver::NodeConfig createNodeConfigFromAppConfig(const uint8_t board_id[8]);
    
    /**
     * @brief Log device identity information
     */
    void logDeviceIdentity() const;
};
