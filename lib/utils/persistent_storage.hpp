/**
 * @file persistent_storage.hpp
 * @brief Persistent storage for frame counter using Pico flash memory
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include <cstdint>

namespace PersistentStorage {

/**
 * @brief Persistent storage manager for frame counter
 * 
 * Uses the last flash sector to store the frame counter value.
 * The storage is wear-leveled by using multiple slots within the sector.
 */
class FrameCounterStorage {
public:
    /**
     * @brief Constructor
     */
    FrameCounterStorage();
    
    /**
     * @brief Initialize the persistent storage
     * @return true if successful
     */
    bool initialize();
    
    /**
     * @brief Read the frame counter from flash
     * @return Current frame counter value, 0 if not found or error
     */
    uint32_t readFrameCounter();
    
    /**
     * @brief Write the frame counter to flash
     * @param counter The frame counter value to store
     * @return true if successful
     */
    bool writeFrameCounter(uint32_t counter);
    
    /**
     * @brief Increment and save the frame counter
     * @return New frame counter value, 0 if error
     */
    uint32_t incrementFrameCounter();

private:
    static constexpr uint32_t FLASH_SIZE = 2 * 1024 * 1024;  // 2MB flash
    static constexpr uint32_t SECTOR_SIZE = 4096;            // 4KB sector
    static constexpr uint32_t STORAGE_SECTOR_OFFSET = FLASH_SIZE - SECTOR_SIZE;  // Last sector
    static constexpr uintptr_t STORAGE_BASE_ADDR = 0x10000000 + STORAGE_SECTOR_OFFSET;  // XIP_BASE + offset
    
    // Storage slot structure
    struct StorageSlot {
        uint32_t magic;          // Magic number for validation
        uint32_t frame_counter;  // Frame counter value
        uint32_t checksum;       // Simple checksum for integrity
        uint32_t reserved;       // Reserved for future use
    };
    
    static constexpr uint32_t MAGIC_NUMBER = 0xDEADBEEF;
    static constexpr uint32_t MAX_SLOTS = SECTOR_SIZE / sizeof(StorageSlot);
    
    /**
     * @brief Calculate checksum for storage slot
     */
    uint32_t calculateChecksum(const StorageSlot& slot) const;
    
    /**
     * @brief Find the latest valid slot
     * @return Slot index, or MAX_SLOTS if none found
     */
    uint32_t findLatestValidSlot() const;
    
    /**
     * @brief Erase the storage sector
     * @return true if successful
     */
    bool eraseSector();
    
    /**
     * @brief Write a storage slot
     * @param slot_index Index of the slot to write
     * @param counter Frame counter value
     * @return true if successful
     */
    bool writeSlot(uint32_t slot_index, uint32_t counter);
    
    bool m_initialized;
    uint32_t m_current_slot;
    uint32_t m_cached_counter;
};

} // namespace PersistentStorage
