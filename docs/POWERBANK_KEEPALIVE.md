# Power Bank Keep-Alive Feature

## Overview

The mioty End-Point Example firmware includes a configurable power bank keep-alive feature to prevent USB power banks from automatically shutting off due to low current consumption.

Many USB power banks (including the Voltcraft PB-19C-M) have an auto-shutoff feature that turns off the output when the connected device draws less than ~60-100mA for an extended period. This feature is designed to save power but can be problematic for low-power IoT devices.

## How It Works

The power bank keep-alive system generates periodic current pulses by driving a GPIO pin high for short durations. This artificially increases the current consumption above the power bank's shutoff threshold, keeping it active.

## Configuration

### Enable the Feature

In `src/config/app_config.hpp`, set:

```cpp
constexpr bool POWER_FROM_POWERBANK = true;
```

### Hardware Configuration Options

#### Option 1: External Resistor (Recommended)

```cpp
constexpr bool USE_EXTERNAL_RESISTOR = true;
```

**Hardware Setup:**
- Connect a 100Ω, 0.25W resistor between GPIO15 (Board::GPIO::POWERBANK_LOAD_PIN) and GND
- This provides ~33mA current draw at 3.3V (safely within 50mA GPIO limit)
- Power dissipation during pulse: ~0.11W

**Pros:**
- Higher current draw (more effective)
- Reliable operation with most power banks

**Cons:**
- Requires external component
- Higher power consumption during pulses

#### Option 2: GPIO Drive Only

```cpp
constexpr bool USE_EXTERNAL_RESISTOR = false;
```

**Hardware Setup:**

- No external components required
- Uses only the GPIO pin's internal drive capability (~20-25mA)
- **100% Safe**: Current is well within RP2040 GPIO specifications (50mA max)

**Pros:**

- No external components needed
- Lower power consumption
- **Zero risk of board damage**
- Simple implementation

**Cons:**

- Lower current draw (may not be sufficient for all power banks)
- Less reliable operation with high-threshold power banks

### Timing Configuration

```cpp
namespace PowerBankKeepAlive {
    constexpr uint32_t PULSE_INTERVAL_MS = 5000;      // Pulse every 5 seconds
    constexpr uint32_t PULSE_DURATION_MS = 150;       // Pulse for 150ms
    constexpr bool ENABLE_LOAD_LED_INDICATOR = true;  // Flash LED during pulses
}
```

## GPIO Pin Assignment

The dummy load GPIO pin is configured in `src/config/board_config.hpp`:

```cpp
namespace Board::GPIO {
    constexpr uint POWERBANK_LOAD_PIN = 15;  // GP15 for dummy load
}
```

The status LED (if enabled) uses the board's default LED pin (GPIO25 on Raspberry Pi Pico).

## Safety Information

### GPIO Drive Only Mode Safety

The GPIO-only mode (without external resistor) is **100% safe** for the RP2040:

- **Current limit**: ~20-25mA (well below 50mA GPIO maximum)
- **Built-in protection**: RP2040 GPIO drivers have internal current limiting
- **Low duty cycle**: 150ms every 5s = 3% duty cycle minimizes thermal stress
- **No damage risk**: Impossible to damage the board in this configuration

### External Resistor Mode Safety

When using an external resistor, ensure proper sizing:

- **Safe option**: 100Ω resistor provides safe ~33mA current (well within 50mA GPIO limit)
- **Alternative**: 150Ω resistor provides ~22mA current (very conservative, may be sufficient)
- **Formula**: Minimum safe resistance = 3.3V / 0.05A = 66Ω (absolute minimum for 50mA)
- **Recommended**: Use 100Ω for good balance of safety and effectiveness

⚠️ **Warning**: Never use resistors smaller than 66Ω to avoid exceeding the 50mA GPIO specification.

## Power Consumption

### With External Resistor
- Normal operation: Baseline device power consumption
- During pulse (150ms every 5s): Additional ~109mW for 150ms (3.3V × 33mA)
- Average additional power: ~3.3mW

### GPIO Drive Only
- Normal operation: Baseline device power consumption  
- During pulse (150ms every 5s): Additional ~66-83mW for 150ms
- Average additional power: ~2-2.5mW

## Troubleshooting

### Power Bank Still Shuts Off

1. **Check connections:** Ensure the resistor is properly connected between GPIO15 and GND
2. **Increase pulse frequency:** Reduce `PULSE_INTERVAL_MS` to 3000ms or 2000ms
3. **Increase pulse duration:** Increase `PULSE_DURATION_MS` to 200ms or 250ms
4. **Use external resistor:** If using GPIO drive only, switch to external resistor mode
5. **Try lower resistance:** If using 150Ω, try 100Ω for higher current (33mA vs 22mA)
6. **Check power bank specs:** Some power banks have very high shutoff thresholds (>50mA)

⚠️ **Note**: Power banks requiring >50mA cannot be safely supported due to RP2040 GPIO limits.

### High Power Consumption

1. **Reduce pulse frequency:** Increase `PULSE_INTERVAL_MS` to 10000ms
2. **Reduce pulse duration:** Decrease `PULSE_DURATION_MS` to 100ms
3. **Use GPIO drive only:** Switch to `USE_EXTERNAL_RESISTOR = false`

### LED Indicator Not Working

1. **Check LED configuration:** Ensure `ENABLE_LOAD_LED_INDICATOR = true`
2. **Verify GPIO:** The LED uses the board's default LED pin (GPIO25 on Pico)

## Compatible Power Banks

The feature has been designed for power banks like:

- Voltcraft PB-19C-M
- Most generic USB power banks with auto-shutoff
- Power banks with ~30-50mA shutoff thresholds

**Note**: Power banks requiring >50mA continuous current cannot be safely supported due to RP2040 GPIO current limitations.

## Disabling the Feature

To disable the power bank keep-alive feature:

```cpp
constexpr bool POWER_FROM_POWERBANK = false;
```

This completely disables the feature with no performance impact.
