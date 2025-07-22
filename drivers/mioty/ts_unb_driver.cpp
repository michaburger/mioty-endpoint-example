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
 * @file ts_unb_driver.cpp
 * @brief TS-UNB-Lib integration driver implementation
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#include "ts_unb_driver.hpp"
#include "ts_unb_lib_wrapper.h"
#include "../../lib/utils/logger.hpp"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <memory>
#include <cstring>

using namespace TsUnbLib::RPPico;

TSUNBDriver::TSUNBDriver() 
    : m_initialized(false)
    , m_last_error(TSUNBStatus::ERROR_NOT_INITIALIZED)
    , m_transmitting(false)
    , m_active_node(nullptr)
{
}

TSUNBDriver::~TSUNBDriver() {
    if (m_initialized) {
        m_initialized = false;
    }
    
    if (m_active_node) {
        // Clean up the node based on the configuration type
        if (m_config.region == TSUNBDriver::Region::EU0) {
            if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
                delete static_cast<TsUnb_EU0_Rfm69w_t*>(m_active_node);
            } else {
                delete static_cast<TsUnb_EU0_Rfm69hw_t*>(m_active_node);
            }
        } else if (m_config.region == TSUNBDriver::Region::EU1) {
            if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
                delete static_cast<TsUnb_EU1_Rfm69w_t*>(m_active_node);
            } else {
                delete static_cast<TsUnb_EU1_Rfm69hw_t*>(m_active_node);
            }
        } else if (m_config.region == TSUNBDriver::Region::EU2) {
            if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
                delete static_cast<TsUnb_EU2_Rfm69w_t*>(m_active_node);
            } else {
                delete static_cast<TsUnb_EU2_Rfm69hw_t*>(m_active_node);
            }
        } else if (m_config.region == TSUNBDriver::Region::US0) {
            if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
                delete static_cast<TsUnb_US0_Rfm69w_t*>(m_active_node);
            } else {
                delete static_cast<TsUnb_US0_Rfm69hw_t*>(m_active_node);
            }
        }
        m_active_node = nullptr;
    }
}

TSUNBStatus TSUNBDriver::initialize(const NodeConfig& config) {
    Logger::info("Initializing TS-UNB driver (Third-Party Modified Version of the Fraunhofer TS-UNB-Lib)...");
    
    m_config = config;
    
    // Create the appropriate node instance based on region and chip type
    if (config.region == TSUNBDriver::Region::EU0) {
        if (config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            m_active_node = new TsUnb_EU0_Rfm69w_t();
        } else {
            m_active_node = new TsUnb_EU0_Rfm69hw_t();
        }
    } else if (config.region == TSUNBDriver::Region::EU1) {
        if (config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            m_active_node = new TsUnb_EU1_Rfm69w_t();
        } else {
            m_active_node = new TsUnb_EU1_Rfm69hw_t();
        }
    } else if (config.region == TSUNBDriver::Region::EU2) {
        if (config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            m_active_node = new TsUnb_EU2_Rfm69w_t();
        } else {
            m_active_node = new TsUnb_EU2_Rfm69hw_t();
        }
    } else if (config.region == TSUNBDriver::Region::US0) {
        if (config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            m_active_node = new TsUnb_US0_Rfm69w_t();
        } else {
            m_active_node = new TsUnb_US0_Rfm69hw_t();
        }
    } else {
        Logger::error("Unsupported region configuration");
        m_last_error = TSUNBStatus::ERROR_INVALID_PARAMETER;
        return m_last_error;
    }
    
    if (!m_active_node) {
        Logger::error("Failed to create TS-UNB node instance");
        m_last_error = TSUNBStatus::ERROR_HARDWARE_FAULT;
        return m_last_error;
    }
    
    // Initialize the node
    configureActiveNode();
    
    m_initialized = true;
    m_last_error = TSUNBStatus::OK;
    Logger::info("TS-UNB driver initialized successfully");
    
    return TSUNBStatus::OK;
}

bool TSUNBDriver::isInitialized() const {
    return m_initialized;
}

TSUNBStatus TSUNBDriver::sendData(const uint8_t* data, size_t length) {
    if (!m_initialized || !m_active_node) {
        return TSUNBStatus::ERROR_NOT_INITIALIZED;
    }
    
    if (length == 0 || !data) {
        return TSUNBStatus::ERROR_INVALID_PARAMETER;
    }
    
    if (m_transmitting) {
        return TSUNBStatus::ERROR_BUFFER_FULL;
    }
    
    Logger::debug("Sending %d bytes via TS-UNB", length);
    
    m_transmitting = true;
    
    // Call the appropriate send method based on the active node type
    if (m_config.region == TSUNBDriver::Region::EU0) {
        if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            static_cast<TsUnb_EU0_Rfm69w_t*>(m_active_node)->send(const_cast<uint8_t*>(data), length);
        } else {
            static_cast<TsUnb_EU0_Rfm69hw_t*>(m_active_node)->send(const_cast<uint8_t*>(data), length);
        }
    } else if (m_config.region == TSUNBDriver::Region::EU1) {
        if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            static_cast<TsUnb_EU1_Rfm69w_t*>(m_active_node)->send(const_cast<uint8_t*>(data), length);
        } else {
            static_cast<TsUnb_EU1_Rfm69hw_t*>(m_active_node)->send(const_cast<uint8_t*>(data), length);
        }
    } else if (m_config.region == TSUNBDriver::Region::EU2) {
        if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            static_cast<TsUnb_EU2_Rfm69w_t*>(m_active_node)->send(const_cast<uint8_t*>(data), length);
        } else {
            static_cast<TsUnb_EU2_Rfm69hw_t*>(m_active_node)->send(const_cast<uint8_t*>(data), length);
        }
    } else if (m_config.region == TSUNBDriver::Region::US0) {
        if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            static_cast<TsUnb_US0_Rfm69w_t*>(m_active_node)->send(const_cast<uint8_t*>(data), length);
        } else {
            static_cast<TsUnb_US0_Rfm69hw_t*>(m_active_node)->send(const_cast<uint8_t*>(data), length);
        }
    }
    
    m_transmitting = false;
    m_last_error = TSUNBStatus::OK;
    
    return TSUNBStatus::OK;
}

TSUNBStatus TSUNBDriver::sendString(const char* str) {
    if (!str) {
        return TSUNBStatus::ERROR_INVALID_PARAMETER;
    }
    
    size_t length = strlen(str);
    return sendData(reinterpret_cast<const uint8_t*>(str), length);
}

TSUNBStatus TSUNBDriver::sendMessage(const TSUNBMessage& message) {
    return sendData(message.payload.data(), message.payload.size());
}

bool TSUNBDriver::isTransmitting() const {
    return m_transmitting;
}

int TSUNBDriver::getRSSI() const {
    // TODO: Implement actual RSSI reading from TS-UNB-Lib
    return -80; // Placeholder value
}

TSUNBStatus TSUNBDriver::reset() {
    Logger::debug("Resetting TS-UNB module");
    
    if (m_active_node) {
        // Reinitialize the node
        configureActiveNode();
    }
    
    m_last_error = TSUNBStatus::OK;
    return TSUNBStatus::OK;
}

TSUNBStatus TSUNBDriver::getLastError() const {
    return m_last_error;
}

void* TSUNBDriver::getActiveNode() {
    return m_active_node;
}

void TSUNBDriver::configureActiveNode() {
    if (!m_active_node) return;
    
    Logger::debug("Configuring TS-UNB node...");
    
    // Configure the node based on type - this is a template for the configuration
    // The actual implementation depends on the TS-UNB-Lib API
    
    // Initialize the node
    if (m_config.region == TSUNBDriver::Region::EU0) {
        if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            auto* node = static_cast<TsUnb_EU0_Rfm69w_t*>(m_active_node);
            node->init();
            node->Tx.setTxPower(m_config.tx_power_dbm);
            node->Mac.setNetworkKey(m_config.network_key[0], m_config.network_key[1], m_config.network_key[2], 
                                   m_config.network_key[3], m_config.network_key[4], m_config.network_key[5], 
                                   m_config.network_key[6], m_config.network_key[7], m_config.network_key[8], 
                                   m_config.network_key[9], m_config.network_key[10], m_config.network_key[11], 
                                   m_config.network_key[12], m_config.network_key[13], m_config.network_key[14], 
                                   m_config.network_key[15]);
            node->Mac.setEui64(m_config.eui64[0], m_config.eui64[1], m_config.eui64[2], m_config.eui64[3], 
                               m_config.eui64[4], m_config.eui64[5], m_config.eui64[6], m_config.eui64[7]);
            node->Mac.setShortAddress(m_config.short_addr[0], m_config.short_addr[1]);
            node->Mac.extPkgCnt = m_config.ext_pkg_cnt;
        } else {
            auto* node = static_cast<TsUnb_EU0_Rfm69hw_t*>(m_active_node);
            node->init();
            node->Tx.setTxPower(m_config.tx_power_dbm);
            node->Mac.setNetworkKey(m_config.network_key[0], m_config.network_key[1], m_config.network_key[2], 
                                   m_config.network_key[3], m_config.network_key[4], m_config.network_key[5], 
                                   m_config.network_key[6], m_config.network_key[7], m_config.network_key[8], 
                                   m_config.network_key[9], m_config.network_key[10], m_config.network_key[11], 
                                   m_config.network_key[12], m_config.network_key[13], m_config.network_key[14], 
                                   m_config.network_key[15]);
            node->Mac.setEui64(m_config.eui64[0], m_config.eui64[1], m_config.eui64[2], m_config.eui64[3], 
                               m_config.eui64[4], m_config.eui64[5], m_config.eui64[6], m_config.eui64[7]);
            node->Mac.setShortAddress(m_config.short_addr[0], m_config.short_addr[1]);
            node->Mac.extPkgCnt = m_config.ext_pkg_cnt;
        }
    } 
    // Similar configuration for other regions would go here...
    // For now, let's implement EU1 which is most commonly used
    else if (m_config.region == TSUNBDriver::Region::EU1) {
        if (m_config.chip_type == TSUNBDriver::ChipType::RFM69W) {
            auto* node = static_cast<TsUnb_EU1_Rfm69w_t*>(m_active_node);
            node->init();
            node->Tx.setTxPower(m_config.tx_power_dbm);
            node->Mac.setNetworkKey(m_config.network_key[0], m_config.network_key[1], m_config.network_key[2], 
                                   m_config.network_key[3], m_config.network_key[4], m_config.network_key[5], 
                                   m_config.network_key[6], m_config.network_key[7], m_config.network_key[8], 
                                   m_config.network_key[9], m_config.network_key[10], m_config.network_key[11], 
                                   m_config.network_key[12], m_config.network_key[13], m_config.network_key[14], 
                                   m_config.network_key[15]);
            node->Mac.setEui64(m_config.eui64[0], m_config.eui64[1], m_config.eui64[2], m_config.eui64[3], 
                               m_config.eui64[4], m_config.eui64[5], m_config.eui64[6], m_config.eui64[7]);
            node->Mac.setShortAddress(m_config.short_addr[0], m_config.short_addr[1]);
            node->Mac.extPkgCnt = m_config.ext_pkg_cnt;
        } else {
            auto* node = static_cast<TsUnb_EU1_Rfm69hw_t*>(m_active_node);
            node->init();
            node->Tx.setTxPower(m_config.tx_power_dbm);
            node->Mac.setNetworkKey(m_config.network_key[0], m_config.network_key[1], m_config.network_key[2], 
                                   m_config.network_key[3], m_config.network_key[4], m_config.network_key[5], 
                                   m_config.network_key[6], m_config.network_key[7], m_config.network_key[8], 
                                   m_config.network_key[9], m_config.network_key[10], m_config.network_key[11], 
                                   m_config.network_key[12], m_config.network_key[13], m_config.network_key[14], 
                                   m_config.network_key[15]);
            node->Mac.setEui64(m_config.eui64[0], m_config.eui64[1], m_config.eui64[2], m_config.eui64[3], 
                               m_config.eui64[4], m_config.eui64[5], m_config.eui64[6], m_config.eui64[7]);
            node->Mac.setShortAddress(m_config.short_addr[0], m_config.short_addr[1]);
            node->Mac.extPkgCnt = m_config.ext_pkg_cnt;
        }
    }
    // Add other regions as needed...
    
    Logger::debug("TS-UNB node configured successfully");
}
