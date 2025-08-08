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
 * @file ts_unb_driver.hpp
 * @brief TS-UNB-Lib integration driver header
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <memory>
#include <vector>
#include <cstdint>

// Include TS-UNB library wrapper to get access to all node types
#include "ts_unb_lib_wrapper.h"

/**
 * @brief TS-UNB communication status
 */
enum class TSUNBStatus {
    OK = 0,
    ERROR_NOT_INITIALIZED,
    ERROR_COMMUNICATION,
    ERROR_TIMEOUT,
    ERROR_INVALID_PARAMETER,
    ERROR_BUFFER_FULL,
    ERROR_HARDWARE_FAULT
};

/**
 * @brief TS-UNB message structure
 */
struct TSUNBMessage {
    std::vector<uint8_t> payload;
    uint32_t timestamp;
    uint8_t message_id;
    
    TSUNBMessage() : timestamp(0), message_id(0) {}
};

/**
 * @brief Driver for TS-UNB-Lib-Pico integration
 * This driver provides a clean interface to the Fraunhofer TS-UNB-Lib
 */
class TSUNBDriver {
public:
    /**
     * @brief Node configuration regions
     */
    enum class Region {
        EU0,     ///< Europe 868 MHz band 0
        EU1,     ///< Europe 868 MHz band 1  
        EU2,     ///< Europe 868 MHz band 2
        US0      ///< US 915 MHz band 0
    };
    
    /**
     * @brief Radio chip types
     */
    enum class ChipType {
        RFM69W,   ///< RFM69W standard version
        RFM69HW   ///< RFM69HW high-power version
    };
    
    /**
     * @brief TS-UNB node configuration
     * Note: Values should be populated from app_config.hpp, no defaults here
     */
    struct NodeConfig {
        Region region;
        ChipType chip_type;
        uint8_t tx_power_dbm;
        uint8_t network_key[16];
        uint8_t eui64[8];
        uint8_t short_addr[2];
        uint32_t ext_pkg_cnt;
    };
    
    TSUNBDriver();
    ~TSUNBDriver();
    
    /**
     * @brief Initialize the TS-UNB driver
     * @param config Node configuration
     * @return TSUNBStatus indicating success or failure
     */
    TSUNBStatus initialize(const NodeConfig& config);
    
    /**
     * @brief Check if driver is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;
    
    /**
     * @brief Send data via TS-UNB
     * @param data Data to send
     * @param length Length of data
     * @return TSUNBStatus indicating success or failure
     */
    TSUNBStatus sendData(const uint8_t* data, size_t length);
    
    /**
     * @brief Send a string via TS-UNB
     * @param str String to send (null-terminated)
     * @return TSUNBStatus indicating success or failure
     */
    TSUNBStatus sendString(const char* str);
    
    /**
     * @brief Send a complete message
     * @param message Message to send
     * @return TSUNBStatus indicating success or failure
     */
    TSUNBStatus sendMessage(const TSUNBMessage& message);
    
    /**
     * @brief Check if transmission is in progress
     * @return true if transmitting, false otherwise
     */
    bool isTransmitting() const;
    
    /**
     * @brief Get signal strength indicator
     * @return RSSI value or -1 if not available
     */
    int getRSSI() const;
    
    /**
     * @brief Reset the TS-UNB module
     * @return TSUNBStatus indicating success or failure
     */
    TSUNBStatus reset();
    
    /**
     * @brief Get last error status
     * @return Last error status
     */
    TSUNBStatus getLastError() const;

    /**
     * @brief Get current frame counter from MAC layer
     * @return Current frame counter value
     */
    uint32_t getFrameCounter() const;

private:
    bool m_initialized;
    NodeConfig m_config;
    TSUNBStatus m_last_error;
    bool m_transmitting;
    
    // TS-UNB node instance - using void* to work with different node types
    void* m_active_node; ///< Pointer to the active node instance
    
    /**
     * @brief Get the active node based on configuration
     * @return Pointer to active node or nullptr
     */
    void* getActiveNode();
    
    /**
     * @brief Configure the active node with settings
     */
    void configureActiveNode();
};
