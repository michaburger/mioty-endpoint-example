/**
 * @file persistent_storage.cpp
 * @brief Persistent storage implementation for frame counter
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#include "persistent_storage.hpp"
#include "logger.hpp"
#include "hardware/flash.h"
#include "pico/critical_section.h"
#include <cstring>

namespace PersistentStorage {

FrameCounterStorage::FrameCounterStorage()
    : m_initialized(false)
    , m_current_slot(0)
    , m_cached_counter(0)
{
}

bool FrameCounterStorage::initialize() {
    if (m_initialized) {
        return true;
    }
    
    Logger::info("Initializing persistent frame counter storage...");
    
    // Find the latest valid slot and read the frame counter
    m_current_slot = findLatestValidSlot();
    
    if (m_current_slot < MAX_SLOTS) {
        // Read the current frame counter
        const StorageSlot* slots = reinterpret_cast<const StorageSlot*>(STORAGE_BASE_ADDR);
        m_cached_counter = slots[m_current_slot].frame_counter;
        Logger::info("Found existing frame counter: %u (slot %u)", m_cached_counter, m_current_slot);
    } else {
        // No valid slots found, start fresh
        m_cached_counter = 0;
        m_current_slot = 0;
        Logger::info("No existing frame counter found, starting from 0");
        
        // Write initial value
        if (!writeSlot(m_current_slot, m_cached_counter)) {
            Logger::error("Failed to write initial frame counter");
            return false;
        }
    }
    
    m_initialized = true;
    Logger::info("Persistent storage initialized successfully");
    return true;
}

uint32_t FrameCounterStorage::readFrameCounter() {
    if (!m_initialized) {
        Logger::warning("Persistent storage not initialized, returning 0");
        return 0;
    }
    
    return m_cached_counter;
}

bool FrameCounterStorage::writeFrameCounter(uint32_t counter) {
    if (!m_initialized) {
        Logger::error("Persistent storage not initialized");
        return false;
    }
    
    // Move to next slot (with wrap-around)
    m_current_slot = (m_current_slot + 1) % MAX_SLOTS;
    
    // If we've wrapped around to slot 0, erase the sector first
    if (m_current_slot == 0) {
        Logger::debug("Wrapping to slot 0, erasing sector for wear leveling");
        if (!eraseSector()) {
            Logger::error("Failed to erase storage sector");
            return false;
        }
    }
    
    if (writeSlot(m_current_slot, counter)) {
        m_cached_counter = counter;
        Logger::debug("Frame counter %u written to slot %u", counter, m_current_slot);
        return true;
    } else {
        Logger::error("Failed to write frame counter");
        return false;
    }
}

uint32_t FrameCounterStorage::incrementFrameCounter() {
    uint32_t new_counter = m_cached_counter + 1;
    
    if (writeFrameCounter(new_counter)) {
        return new_counter;
    } else {
        Logger::error("Failed to increment frame counter");
        return 0;
    }
}

uint32_t FrameCounterStorage::calculateChecksum(const StorageSlot& slot) const {
    // Simple checksum: XOR of magic and frame_counter
    return slot.magic ^ slot.frame_counter;
}

uint32_t FrameCounterStorage::findLatestValidSlot() const {
    const StorageSlot* slots = reinterpret_cast<const StorageSlot*>(STORAGE_BASE_ADDR);
    uint32_t latest_slot = MAX_SLOTS;  // Invalid slot index
    uint32_t highest_counter = 0;
    
    for (uint32_t i = 0; i < MAX_SLOTS; i++) {
        // Check if slot has valid magic number
        if (slots[i].magic == MAGIC_NUMBER) {
            // Verify checksum
            uint32_t expected_checksum = calculateChecksum(slots[i]);
            if (slots[i].checksum == expected_checksum) {
                // Valid slot found, check if it has the highest counter
                if (slots[i].frame_counter >= highest_counter) {
                    highest_counter = slots[i].frame_counter;
                    latest_slot = i;
                }
            } else {
                Logger::warning("Slot %u has invalid checksum", i);
            }
        }
    }
    
    return latest_slot;
}

bool FrameCounterStorage::eraseSector() {
    Logger::debug("Erasing storage sector...");
    
    // Create a critical section for flash operation
    critical_section_t crit_sec;
    critical_section_init(&crit_sec);
    critical_section_enter_blocking(&crit_sec);
    
    // Erase the sector
    flash_range_erase(STORAGE_SECTOR_OFFSET, SECTOR_SIZE);
    
    // Exit critical section
    critical_section_exit(&crit_sec);
    critical_section_deinit(&crit_sec);
    
    Logger::debug("Storage sector erased");
    return true;
}

bool FrameCounterStorage::writeSlot(uint32_t slot_index, uint32_t counter) {
    if (slot_index >= MAX_SLOTS) {
        Logger::error("Invalid slot index: %u", slot_index);
        return false;
    }
    
    // Prepare the slot data
    StorageSlot slot;
    slot.magic = MAGIC_NUMBER;
    slot.frame_counter = counter;
    slot.checksum = calculateChecksum(slot);
    slot.reserved = 0;
    
    // Calculate the offset for this slot
    uint32_t slot_offset = STORAGE_SECTOR_OFFSET + (slot_index * sizeof(StorageSlot));
    
    Logger::debug("Writing slot %u at offset 0x%08X", slot_index, slot_offset);
    
    // Create a critical section for flash operation
    critical_section_t crit_sec;
    critical_section_init(&crit_sec);
    critical_section_enter_blocking(&crit_sec);
    
    // Write the slot data
    flash_range_program(slot_offset, reinterpret_cast<const uint8_t*>(&slot), sizeof(StorageSlot));
    
    // Exit critical section
    critical_section_exit(&crit_sec);
    critical_section_deinit(&crit_sec);
    
    // Verify the write by reading back
    const StorageSlot* written_slot = reinterpret_cast<const StorageSlot*>(STORAGE_BASE_ADDR + (slot_index * sizeof(StorageSlot)));
    
    if (written_slot->magic == MAGIC_NUMBER && 
        written_slot->frame_counter == counter &&
        written_slot->checksum == slot.checksum) {
        Logger::debug("Slot %u written and verified successfully", slot_index);
        return true;
    } else {
        Logger::error("Slot %u write verification failed", slot_index);
        return false;
    }
}

} // namespace PersistentStorage
