# mioty Generic Node Framework

## 🌐 What is mioty?

mioty (short for "massive-IoT" and also "my-IoT") is a revolutionary Low Power Wide Area Network (LPWAN) technology initially developed by Fraunhofer IIS. Today, mioty is promoted and advanced by the **mioty Alliance** and its members - a global community of companies, institutes, and developers working together to create the most accessible, robust, and efficient IoT connectivity solution on the market.

**Why mioty is perfect for IoT projects:**

- **Long Range**: Up to 15km in rural areas, 3-5km in cities
- **Ultra Low Power**: Devices can run for years on a single battery  
- **Massive Capacity**: Supports 1 million+ daily telegrams per base station
- **Robust Communication**: Works reliably even in challenging RF environments
- **Cost Effective**: Less base stations needed to deploy your own network

Unlike other LPWAN technologies, mioty uses a unique telegram splitting approach that makes it extremely robust against interference and allows for truly massive device deployments. 

## 🤝 About the mioty Alliance

The **mioty Alliance** is a global ecosystem of technology leaders, hardware manufacturers, system integrators, and service companies united by a shared vision: enabling the most accessible, robust, and efficient massive IoT connectivity solution.

The Alliance provides an open, standardized, and interoperable ecosystem across the entire IoT value chain - from end-point devices and base stations to IoT platforms and applications. This collaborative approach ensures that mioty technology continues to evolve and remains future-proof for all users.

## 🎯 The mioty Generic Node Concept

This project provides a **low-cost, open-source starting point** for your mioty IoT projects. Built around affordable hardware for less than **$10**, it combines:

- **Raspberry Pi Pico** (~$3) - Powerful RP2040 microcontroller
- **HopeRF RFM69HW** (~$2) - 868MHz radio module for mioty communication
- **Open Source TS-UNB Library** from Fraunhofer - Free for maker projects

⚠️ **Important Note**: This open-source project is designed for **maker and educational purposes**. While it uses Fraunhofer's open-source TS-UNB library, production-grade applications should consider the commercial libraries available from Fraunhofer and Stackforce, which offer enhanced features, support, and quality assurance.

## 🔧 Modular Platform Vision

This framework is designed as a **modular foundation** that can grow with your project needs:

- **Core Board**: Raspberry Pi Pico off-the-shelf
- **RF Board**: Include the RFM69HW module or any other module if you want to port the TS-UNB library
- **Sensor Boards**: Add temperature, humidity, pressure, light sensors
- **GPS Board**: For location tracking applications  
- **Power Board**: Solar charging, battery management
- **Display Board**: OLED/LCD for local status indication
- **Custom Boards**: Design your own for specific applications

The software architecture supports this modularity, making it easy to add new sensors and features without starting from scratch.

## 🚀 Key Features

- **🏗️ Modular Design**: Ready to expand with additional sensors and hardware
- **📡 mioty Integration**: Full TS-UNB protocol support with RFM69HW
- **📊 Smart Payloads**: Efficient binary data transmission
- **🌡️ Sensor Ready**: Built-in temperature sensor with easy expansion
- **🔧 Beginner Friendly**: Clear structure and comprehensive documentation

## 📁 What's Inside

The project is organized to make it easy to understand and extend:

```
mioty-generic-node-fw/
├── src/                          # Your main application
│   ├── main.cpp                  # Program starts here
│   ├── app/                      # Main application logic
│   └── config/                   # All settings in one place
├── drivers/                      # Hardware drivers
│   ├── mioty/                    # mioty radio communication
│   └── sensors/                  # Temperature, humidity, etc.
├── lib/                          # Core libraries
│   ├── utils/                    # Helper functions
│   └── ts-unb-lib-rfm69/         # mioty protocol library
└── docs/                         # Documentation
```

This structure is designed to grow with your project - adding new sensors or features is straightforward and won't break existing code.

## 🎯 Getting Started

### What You'll Need

**Hardware:**

- Raspberry Pi Pico (any variant with RP2040)
- RFM69HW radio module (868MHz for Europe, 915MHz for US)
- Breadboard and jumper wires for prototyping
- USB cable for programming and power

**Software:**

- Visual Studio Code (recommended)
- Raspberry Pi Pico extension for VS Code
- Git for downloading the project

**Optional but Helpful:**

- Basic soldering kit for permanent connections
- Multimeter for troubleshooting
- mioty base station for testing, for example [miotyGO](https://github.com/loriot/miotyGO/tree/master)

### Hardware Connections

Connect your RFM69HW module to the Raspberry Pi Pico following this simple wiring diagram:

| RFM69HW Pin | Pico GPIO | Wire Color | Purpose |
|-------------|-----------|------------|---------|
| SCK | GP18 | Yellow | SPI Clock |
| MOSI | GP19 | Blue | Data to Radio |
| MISO | GP16 | Green | Data from Radio |
| NSS | GP17 | Orange | Chip Select |
| RESET | GP22 | Red | Reset Radio |
| GND | GND | Black | Ground |
| 3.3V | 3V3 | Red | Power |

💡 **Beginner Tip**: Use a breadboard for your first setup. Once everything works, you can create a more permanent solution with a custom PCB or perfboard.

### Software Setup

1. **Get the code:**

```bash
git clone <your-repo-url>
cd mioty-generic-node-fw
```

2. **Open in VS Code:**
   - Install the "Raspberry Pi Pico" extension
   - Open the project folder
   - The extension will guide you through SDK setup

3. **Build the project:**
   - Press `Ctrl+Shift+P` and select "Raspberry Pi Pico: Configure CMake"
   - Press `Ctrl+Shift+P` and select "Raspberry Pi Pico: Compile"

4. **Flash to your Pico:**
   - Hold the BOOTSEL button on your Pico
   - Connect the USB cable
   - Release BOOTSEL (Pico appears as USB drive)
   - Copy the generated `.uf2` file to the Pico drive
   - The Pico will restart automatically

### 📺 Monitoring Your Device

Once your device is running, you can monitor its activity and debug any issues using Visual Studio Code's built-in Serial Monitor:

1. **Install the Serial Monitor extension:**
   - In VS Code, go to Extensions (`Ctrl+Shift+X`)
   - Search for "Serial Monitor" by Microsoft
   - Install the official extension

2. **Start monitoring:**
   - Press `Ctrl+Shift+P` and select "Serial Monitor: Start Monitoring"
   - Select your Pico's COM port (usually shows as "Board CDC" or similar)
   - Set baud rate to 115200
   - You'll see real-time logs showing:
     - Sensor readings and calibration
     - mioty transmission attempts and results
     - System status and error messages
     - Power management and sleep cycles

💡 **Debugging Tip**: The logs will help you understand what your device is doing and troubleshoot any connectivity or sensor issues. Look for transmission confirmations and any error messages.

🎉 **Success!** Your mioty node should now be running and sending temperature data every 60 seconds.

## 📡 How Data is Sent (mioty Payloads)

Instead of sending text messages, this system sends compact binary data to save power and bandwidth. Think of it like sending a postcard instead of a letter - it contains exactly the information needed in the smallest possible space.

### What Gets Sent

Every transmission contains:

1. **Device Information** (8 bytes): Who sent it, firmware version, why it was sent
2. **Sensor Data** (variable): Temperature, humidity, or whatever sensors you add

### Example: Temperature Reading

When your device measures 23.45°C, it sends something like this:

```
01 01 00 01 14 01 00 00 29 09
```

That's what we call the payload. More details about what this means later.

### Adding More Sensors

The beauty of this system is that adding new sensors is straightforward. Want to add humidity? Just:

1. Connect the sensor hardware
2. Add a few lines to the configuration 
3. The system automatically includes it in transmissions

The framework is designed so you don't need to understand the low-level details to add new features.

## 📊 Understanding mioty Payload Format & Blueprints

While the system handles most complexity automatically, understanding the payload structure helps when adding sensors or integrating with backend systems.

### Current Payload Structure

Every mioty transmission from this framework follows a standardized 10-byte format:

```
[8-byte Header][2-byte Temperature Data]
```

#### Complete Payload Breakdown

| Bytes | Field | Description | Example Value |
|-------|-------|-------------|---------------|
| 0 | Payload Version | Structure version | `0x01` |
| 1-2 | Firmware Version | Major.Minor (2 bytes) | `0x01 0x00` (v1.0) |
| 3 | Hardware Version | Board revision | `0x01` |
| 4 | TX Power | Transmission power (dBm) | `0x14` (20 dBm) |
| 5 | Trigger Type | What caused transmission | `0x01` (Timer) |
| 6-7 | Reserved | Future use | `0x00 0x00` |
| 8-9 | Temperature | Sensor data (scaled x100) | `0x29 0x09` (23.45°C) |

#### Example Complete Payload

```
01 01 00 01 14 01 00 00 29 09
└─────── Header ────────┘└─Temp─┘
```

Where temperature `23.45°C` becomes `2345` (scaled by 100) = `0x2909` in big-endian format.

### mioty Blueprint for Current Firmware

Here's the complete blueprint that describes how to decode the entire 10-byte payload:

```json
{
  "version": "1.0",
  "typeEui": "70b3d56770000001",
  "meta": {
    "name": "mioty Generic Node - Temperature Only",
    "vendor": "mioty Alliance",
    "description": "RP2040 + RFM69HW with internal temperature sensor"
  },
  "uplink": [
    {
      "id": 0,
      "crypto": 0,
      "payload": [
        {
          "name": "payload_version",
          "component": "8bitUnsigned"
        },
        {
          "name": "firmware_major",
          "component": "8bitUnsigned"
        },
        {
          "name": "firmware_minor",
          "component": "8bitUnsigned"
        },
        {
          "name": "hardware_version",
          "component": "8bitUnsigned"
        },
        {
          "name": "tx_power_dbm",
          "component": "8bitSigned"
        },
        {
          "name": "trigger_type",
          "component": "8bitUnsigned"
        },
        {
          "name": "reserved1",
          "component": "8bitUnsigned"
        },
        {
          "name": "reserved2",
          "component": "8bitUnsigned"
        },
        {
          "name": "temperature",
          "component": "16bitTemperature"
        }
      ]
    }
  ],
  "component": {
    "8bitUnsigned": {
      "size": 8,
      "type": "int",
      "func": "$",
      "unit": "",
      "littleEndian": false
    },
    "8bitSigned": {
      "size": 8,
      "type": "int",
      "func": "$",
      "unit": "dBm",
      "littleEndian": false
    },
    "16bitTemperature": {
      "size": 16,
      "type": "int",
      "func": "$/100",
      "unit": "°C",
      "littleEndian": false
    }
  }
}
```

This blueprint tells the mioty base station exactly how to decode all 10 bytes of the payload, providing both header information and sensor data in a structured format.

### Future Payload Extensions

The framework is designed to easily accommodate additional sensors. Here's how the payload would evolve:

#### Adding Humidity Sensor (Future)

```
[8-byte Header][2-byte Temperature][2-byte Humidity] = 12 bytes total
```

#### Adding Multiple Sensors (Future)

```
[8-byte Header][Temperature][Humidity][Pressure][Battery] = 16 bytes total
```

When adding sensors:

1. **Sensor data always follows the 8-byte header**
2. **Order matters** - sensors must be added in the same sequence in both firmware and blueprint
3. **New TypeEUI required** - each different payload structure needs its own unique identifier
4. **Blueprint components expand** - new sensor types get added to the component definitions

#### Key Design Principles

- **Header stays constant** - The 8-byte header format never changes, ensuring compatibility
- **Sensors append sequentially** - New sensor data is added after existing sensors
- **Scaling preserved** - All sensors use consistent scaling (typically x100 for decimal precision)
- **Big-endian format** - Multi-byte values use most significant byte first

This approach ensures that as you add sensors to your project, the base station can properly decode and display all collected data while maintaining backward compatibility with the header structure.

## ⚙️ Customizing Your Node

### Basic Configuration

All the important settings are in easy-to-find configuration files:

- **Hardware Settings** (`board_config.hpp`): Which pins connect to what
- **Application Settings** (`app_config.hpp`): How often to send data, transmission power
- **Sensor Settings** (`payload_config.hpp`): What data to include

### Adding New Sensors

The system is designed to make adding sensors simple:

1. **Connect your sensor** to the Pico (I2C, SPI, or analog)
2. **Add a driver** (we provide templates and examples)
3. **Update the configuration** to include your sensor data
4. **Rebuild and flash** - your new sensor is now part of the system

### Example: Adding a Humidity Sensor

```cpp
// In payload_config.hpp, add:
HUMIDITY = 0x03,  // New sensor type

// In your application:
humidity_sensor.read();
payload_builder.addSensorData(SensorType::HUMIDITY, humidity_value);
```

The system handles everything else automatically.

## 🔧 Expanding Your Project

The framework is designed to grow with your needs. The structure is clean and modular, making it easy to add new features without breaking existing functionality.

### Adding New Sensors

Each sensor in the system has:

- A **driver** that talks to the hardware
- **configuration** that defines how data is formatted  
- **integration** into the main application loop

We provide examples and templates to help you add common sensors like:

- Humidity and pressure sensors (BME280, SHT30)
- GPS modules for location tracking
- Battery voltage monitoring
- Light sensors and accelerometers

### Power Management

Even though the RP2040 is not the most low-power chip on the market, it can be optimized for battery operation:

- **Sleep modes** between transmissions to save power
- **Efficient transmission** protocols to minimize radio usage
- **Configurable intervals** - send data every few minutes or once per day
- **Low power sensors** that don't drain your battery

### Creating Custom Hardware

The modular design makes it perfect for custom boards:

- **Stack modules** on top of the Pico for clean installations  
- **Custom breakout boards** for specific sensor combinations
- **Enclosure-friendly** layouts for outdoor deployments
- **Expansion headers** for adding more features later

## 🚀 What's Next?

This project gives you everything needed to start building mioty-connected devices. Here are some ideas for what you could build:

- **Environmental monitoring**: Temperature, humidity, air quality sensors
- **Asset tracking**: GPS + mioty for tracking vehicles, equipment, or livestock  
- **Smart agriculture**: Soil moisture, temperature, and light monitoring
- **Building automation**: Room sensors, door/window status, occupancy detection
- **Industrial monitoring**: Vibration, pressure, or equipment status sensors

The key advantage is **long range and low power** - your devices can operate for years on batteries while communicating over kilometers.

## 📄 License & Support

This project uses a dual-license structure:

- **Framework Code**: MIT License © 2025 mioty Alliance e.V.
- **TS-UNB-Lib**: Fraunhofer TS-UNB-Lib License (academic/non-commercial use)

The Fraunhofer TS-UNB-Lib is included as a third-party library with its original licensing terms. For commercial use of the mioty protocol, contact Sisvel International S.A. for patent licensing.

## 📞 Contact & Resources

**mioty Alliance e.V.**  
Technical Contact: Micha Burger  
Email: micha.burger@mioty-alliance.com

**Learn More:**

- [mioty Alliance Website](https://mioty-alliance.com)
- [TS-UNB Protocol Documentation](https://www.etsi.org/deliver/etsi_ts/103300_103399/103357/)
- [Raspberry Pi Pico Documentation](https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf)

**Community:**

- Report issues via GitHub Issues
- Feature requests welcome
- Pull requests encouraged

## 🤖 Acknowledgments

This project has been developed with the invaluable assistance of **Claude Sonnet 4**, which helped design the architecture, write documentation, and ensure code quality throughout the development process.

---

*Built with ❤️ for the mioty IoT ecosystem*
