# Persistent Frame Counter Implementation

This document describes the implementation of persistent frame counter storage for the mioty endpoint example. The frame counter is crucial for mioty networks as it provides replay protection and ensures message uniqueness.

## Overview

The mioty protocol uses an extended packet counter (`extPkgCnt`) that increments with each transmission. Previously, this counter would reset to 0 every time the MCU restarted, which could cause issues in production deployments:

1. **Replay Protection**: Networks could reject messages if they appear to be replays
2. **Message Tracking**: Base stations might lose track of message sequence
3. **Security**: Frame counter resets could potentially be exploited

## Solution: Flash-Based Persistent Storage

The implementation uses the Raspberry Pi Pico's flash memory to persistently store the frame counter across reboots.

### Key Features

- **Wear Leveling**: Uses multiple slots within a flash sector to distribute writes
- **Data Integrity**: Includes magic numbers and checksums for validation
- **Automatic Recovery**: Finds and loads the latest valid frame counter on startup
- **Minimal Flash Usage**: Uses only the last 4KB sector of flash memory

### Architecture

#### Storage Layout

```
Flash Sector (4KB at end of flash):
├── Slot 0:  [Magic][Counter][Checksum][Reserved]
├── Slot 1:  [Magic][Counter][Checksum][Reserved]
├── ...
└── Slot N:  [Magic][Counter][Checksum][Reserved]
```

Each slot is 16 bytes, allowing ~256 slots per sector.

#### Wear Leveling Strategy

1. **Sequential Writing**: Each update uses the next slot in sequence
2. **Sector Recycling**: When all slots are used, the sector is erased and writing starts from slot 0
3. **Write Distribution**: Spreads writes across the entire sector to prevent wear concentration

### Implementation Details

#### Files Added

- `lib/utils/persistent_storage.hpp` - Header file with class definition
- `lib/utils/persistent_storage.cpp` - Implementation of persistent storage
- Updates to `src/app/application.cpp` - Integration with application logic
- Updates to `drivers/mioty/ts_unb_driver.cpp` - Added frame counter access method

#### Integration Points

1. **Initialization**: Frame counter loaded from flash during startup
2. **Transmission**: Counter saved to flash after successful transmission
3. **Recovery**: Automatic detection and loading of latest valid counter

#### Storage Format

```cpp
struct StorageSlot {
    uint32_t magic;          // 0xDEADBEEF for validation
    uint32_t frame_counter;  // The actual frame counter value
    uint32_t checksum;       // XOR of magic and frame_counter
    uint32_t reserved;       // Reserved for future use
};
```

### Usage

The persistent frame counter is automatically managed by the application. No user intervention is required.

#### Startup Behavior

```
INFO: Initializing persistent frame counter storage...
INFO: Found existing frame counter: 1234 (slot 15)
INFO: Persistent storage initialized successfully
INFO: Loaded frame counter from persistent storage: 1234
```

#### Transmission Behavior

```
INFO: ✓ MIOTY transmission successful (packet #42)
DEBUG: Frame counter saved to persistent storage: 1235
```

#### First Run Behavior

```
INFO: Initializing persistent frame counter storage...
INFO: No existing frame counter found, starting from 0
INFO: Persistent storage initialized successfully
INFO: Loaded frame counter from persistent storage: 0
```

### Configuration

#### Memory Usage

- **Flash Usage**: 4KB (last sector of flash)
- **RAM Usage**: Minimal (~32 bytes for cached values)
- **Write Cycles**: ~256 writes per sector erase

#### Flash Considerations

- **Endurance**: RP2040 flash typically supports 10,000-100,000 erase cycles
- **Sector Size**: 4KB sectors are standard for the flash used on RP2040
- **Write Granularity**: Flash can be programmed in 256-byte pages

#### Wear Leveling Calculation

With 256 slots per sector and ~10,000 erase cycles:
- Total writes before wear-out: 256 × 10,000 = 2,560,000 transmissions
- At 1 transmission/minute: ~4.9 years of operation
- At 1 transmission/hour: ~292 years of operation

### Error Handling

#### Corruption Detection

- **Magic Number Check**: Validates slot integrity
- **Checksum Verification**: Detects data corruption
- **Multiple Slot Validation**: Finds highest valid counter if some slots are corrupted

#### Recovery Strategies

1. **Partial Corruption**: Uses highest valid counter found
2. **Total Corruption**: Starts fresh from counter 0
3. **Flash Failure**: Gracefully degrades (counter resets on reboot)

#### Logging

The implementation provides detailed logging for debugging:

```cpp
DEBUG: Writing slot 15 at offset 0x001FF0F0
DEBUG: Slot 15 written and verified successfully
DEBUG: Frame counter saved to persistent storage: 1235
```

### Testing

#### Verification Steps

1. **Power Cycle Test**: Verify counter persists across reboots
2. **Corruption Test**: Manually corrupt flash and verify recovery
3. **Wear Test**: Verify wear leveling with multiple transmissions
4. **Edge Case Test**: Test behavior at sector boundaries

#### Manual Testing

```bash
# Build and flash the firmware
ninja -C build
picotool load mioty_endpoint_example.uf2

# Monitor serial output to verify counter persistence
# Power cycle the device and observe that counter continues from last value
```

### Troubleshooting

#### Common Issues

1. **Counter Resets to 0**: Check flash integrity and logs
2. **Write Failures**: Verify flash is not write-protected
3. **Performance Issues**: Monitor flash write frequency

#### Debug Output

Enable debug logging to see detailed storage operations:

```cpp
// In app_config.hpp
constexpr bool ENABLE_DEBUG_OUTPUT = true;
```

#### Flash Analysis

Use picotool to examine flash contents:

```bash
# Read flash sector
picotool save -r 0x101FF000 0x10200000 flash_sector.bin

# Analyze with hex editor to verify data structure
```

### Future Enhancements

#### Potential Improvements

1. **Compression**: Pack multiple counters in single slot
2. **Encryption**: Encrypt stored data for security
3. **Backup**: Use multiple sectors for redundancy
4. **Statistics**: Track write frequency and flash health

#### Configuration Options

Consider adding configuration for:
- Storage sector location
- Wear leveling aggressiveness
- Backup redundancy level
- Health monitoring

### Security Considerations

#### Frame Counter Security

- **Monotonic Requirement**: Counter must never decrease
- **Replay Protection**: Networks reject old counter values
- **Synchronization**: Large counter jumps may require resynchronization

#### Implementation Security

- **Flash Protection**: Consider enabling flash write protection in production
- **Validation**: Always validate stored data before use
- **Fallback**: Graceful handling of storage failures

### Performance Impact

#### Runtime Performance

- **Startup**: +~10ms for storage initialization
- **Transmission**: +~5ms for flash write operation
- **Memory**: Minimal RAM usage for caching

#### Flash Longevity

- **Write Frequency**: One write per transmission
- **Wear Distribution**: Even wear across 256 slots
- **Lifecycle**: Designed for multi-year operation

This persistent frame counter implementation ensures reliable operation in production mioty networks while maintaining excellent performance and flash longevity.
