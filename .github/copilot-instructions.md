# AI Coding Agent Instructions

## Project Overview
This is a PlatformIO-based embedded project for **Seeed XIAO ESP32-S3** implementing a **SenseAI security camera** with motion detection and Twilio alert integration. The system uses **BLE for device provisioning** and **WiFi for connectivity**. The project uses the Arduino framework on ESP32 hardware.

## Hardware Platform
- **Board**: Seeed XIAO ESP32-S3 (dual-core Xtensa LX7, WiFi/BLE, camera support)
- **Platform**: Custom ESP32 platform from pioarduino (not official Espressif platform)
- **Framework**: Arduino
- **Connectivity**: WiFi 2.4GHz (802.11 b/g/n) + Bluetooth 5.0 (BLE only - no Zigbee/Thread support)

## Development Workflow

### Building & Uploading
```bash
# Build the project
pio run

# Upload to board (ensure board is connected via USB)
pio run --target upload

# Build and upload in one command
pio run -t upload

# Monitor serial output
pio device monitor

# Clean build artifacts
pio run --target clean
```

### PlatformIO Commands
- Use `pio` CLI commands, not Arduino IDE
- The `.pio/` directory contains build artifacts (ignored in git)
- VSCode uses the PlatformIO IDE extension (`pioarduino.pioarduino-ide`)

## Project Structure

```
platformio.ini      # Board config, dependencies, build settings
src/main.cpp        # Main application entry point
include/            # Project header files (.h)
lib/                # Private/custom libraries (each in own subdirectory)
test/               # Unit tests
```

## Key Conventions

### Code Organization
- Main application logic goes in `src/main.cpp`
- Header files for project code belong in `include/`
- Custom libraries go in `lib/<LibraryName>/` with their own `.h` and `.cpp` files
- PlatformIO auto-discovers library dependencies from `#include` statements

### Arduino Framework Patterns
- `setup()` runs once at startup - use for initialization
- `loop()` runs continuously - keep non-blocking
- Use Arduino APIs: `Serial`, `WiFi`, `delay()`, etc.

### ESP32-Specific Considerations
- This board has **camera support** - likely OV2640 or similar via PSRAM
- WiFi credentials typically hardcoded or stored in SPIFFS/EEPROM
- Serial baud rate commonly 115200 for `Serial.begin()`
- ESP32 has dual cores - Arduino framework runs on Core 1 by default

## Dependencies & Libraries
- Add libraries to `platformio.ini` under `lib_deps`
- Example: `lib_deps = WiFi, HTTPClient, ArduinoJson`
- PlatformIO automatically downloads and manages library versions

## Debugging
- Use `Serial.print()` statements for debugging
- Monitor with `pio device monitor` (default baud: 115200)
- Check `.pio/build/seeed_xiao_esp32s3/` for compilation errors
- Board must be in bootloader mode for upload (typically auto-detected)
## Architecture & System States

### State Machine
The system operates in three states:
1. **STATE_BLE_PROVISIONING**: Advertising BLE for WiFi credential setup (5 min timeout)
2. **STATE_WIFI_CONNECTING**: Connecting to WiFi with stored credentials (20 sec timeout)
3. **STATE_RUNNING**: Main operation - camera capture, AI inference, Twilio alerts

### Configuration Flow
1. Device boots → Check for stored WiFi credentials in Preferences
2. If no credentials → Start BLE provisioning service
3. Mobile app writes SSID/password via BLE characteristics
4. Credentials saved to NVS (persistent storage)
5. Connect to WiFi → Initialize camera → Start AI processing

### Key Components
- **BLE Provisioning** (`ble_provisioning.cpp`): NimBLE-based credential exchange
- **SenseAI** (`sense_ai.cpp`): Motion detection + AI inference placeholder
- **Alert System**: HTTP POST with JSON payload containing base64-encoded JPEG
- **Camera Config** (`config.h`): XIAO ESP32-S3 Sense pin mappings

## Common Tasks for This Project

### BLE Provisioning
Device advertises as `SenseAI_Camera` with service UUID `4fafc201-1fb5-459e-8fcc-c5c9c331914b`:
- Write SSID to characteristic `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Write password to characteristic `1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e`
## Important Notes
- **Do NOT** modify `.pio/` or `.vscode/c_cpp_properties.json` (auto-generated)
- Platform URL in `platformio.ini` uses pioarduino fork, not official Espressif
- **ESP32-S3 has NO Zigbee/Thread support** - only WiFi + BLE (use ESP32-H2/C6 for 802.15.4)
- Camera pin definitions in `config.h` are specific to XIAO ESP32-S3 Sense
- Credentials persist across reboots via `Preferences` (NVS storage)
- BLE stops after successful WiFi connection to free memory
- Motion detection is CPU-based; integrate TFLite for advanced AI models
- Triggers alert if >5% of pixels change by >30 intensity units
- AI inference confidence threshold: 0.7

### Alert Configuration
Set HTTP endpoint in `config.h`:
```cpp
#define ALERT_ENDPOINT "https://your-server.com/api/alerts"
#define ALERT_AUTH_TOKEN "your_api_key"  // Optional
```

Alert payload format (JSON):
```json
{
  "timestamp": 123456,
  "device": "SenseAI_Camera",
  "event": "motion_detected",
  "confidence": 0.85,
  "image": "data:image/jpeg;base64,..."
}
```
- Check Seeed documentation for specific camera pins

## Important Notes
- **Do NOT** modify `.pio/` or `.vscode/c_cpp_properties.json` (auto-generated)
- Platform URL in `platformio.ini` uses pioarduino fork, not official Espressif
- This board differs from standard ESP32-CAM - verify pin mappings
- Current `main.cpp` is boilerplate - actual implementation pending
