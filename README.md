# SenseAI Security Camera

ESP32-S3 based intelligent security camera with motion detection, AI inference, and Twilio alert integration.

## Features

- 🔒 **BLE Provisioning**: Easy WiFi setup via Bluetooth Low Energy
- 📷 **Camera Integration**: OV2640 camera on Seeed XIAO ESP32-S3 Sense
- 🧠 **Motion Detection**: Pixel-difference based motion sensing
- 🤖 **AI-Ready**: Placeholder for TensorFlow Lite integration
- 📱 **Twilio Alerts**: MMS notifications with captured images
- 💾 **Persistent Storage**: WiFi credentials saved across reboots

## Hardware Requirements

- **Board**: [Seeed XIAO ESP32-S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- **Camera**: Built-in OV2640 (included with Sense variant)
- **Power**: USB-C or 3.7V LiPo battery

### Why Not Zigbee/Thread?

The ESP32-S3 **does not support Zigbee or Thread** as these protocols require 802.15.4 radio hardware (only available on ESP32-H2 and ESP32-C6). This project uses:
- **BLE** for device provisioning
- **WiFi** for main connectivity and cloud communication

## Quick Start

### 1. Install Dependencies

```bash
# Install PlatformIO CLI (if not already installed)
pip install platformio

# Clone and navigate to project
cd SecurityCameraTwilio
```

### 2. Configure Twilio

Edit `include/config.h` and add your Twilio credentials:

```cpp
#define TWILIO_ACCOUNT_SID "ACxxxxxxxxxxxxx"
#define TWILIO_AUTH_TOKEN "your_auth_token"
#define TWILIO_FROM_NUMBER "+1234567890"
#define TWILIO_TO_NUMBER "+0987654321"
```

### 3. Build & Upload

```bash
# Build the project
pio run

# Upload to board (connect via USB-C)
pio run -t upload

# Monitor serial output
pio device monitor
```

## Device Provisioning

### First Boot (No WiFi Credentials)

1. Device starts BLE advertising as `SenseAI_Camera`
2. Connect using a BLE mobile app or custom tool
3. Write credentials to BLE characteristics:
   - **SSID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
   - **Password**: `1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e`
4. Device connects to WiFi and saves credentials

### Subsequent Boots

Device automatically connects using stored credentials. To reset:

```cpp
// Add to setup() to clear stored credentials
preferences.clear();
```

## Project Structure

```
include/
  ├── config.h              # Pin mappings, thresholds, API keys
  ├── ble_provisioning.h    # BLE service for WiFi setup
  └── sense_ai.h            # Camera & AI inference logic

src/
  ├── main.cpp              # State machine & main loop
  ├── ble_provisioning.cpp  # BLE characteristic handlers
  └── sense_ai.cpp          # Motion detection & Twilio alerts

platformio.ini              # Build config & dependencies
```

## System Architecture

### State Machine

```
┌─────────────────────┐
│ BLE_PROVISIONING    │ ──[Credentials Received]──┐
│ (Advertising)       │                            │
└─────────────────────┘                            ▼
                                         ┌─────────────────────┐
                                         │ WIFI_CONNECTING     │
                                         │ (Connecting to AP)  │
                                         └─────────────────────┘
                                                    │
                                    [Connected]     │
                                                    ▼
                                         ┌─────────────────────┐
                                         │ RUNNING             │
                                         │ (Camera Active)     │
                                         └─────────────────────┘
```

### Motion Detection Flow

1. Capture frame from camera (320x240 JPEG)
2. Compare with previous frame pixel-by-pixel
3. Calculate percentage of changed pixels
4. If >5% change → Motion detected
5. Run AI inference (placeholder)
6. If confidence >0.7 → Send Twilio alert with image

## Configuration

### Camera Settings (`sense_ai.cpp`)

```cpp
config.frame_size = FRAMESIZE_QVGA;  // 320x240
config.pixel_format = PIXFORMAT_JPEG;
config.jpeg_quality = 12;  // 10-63 (lower = better quality)
config.fb_count = 2;  // Frame buffers in PSRAM
```

### Detection Thresholds (`config.h`)

```cpp
#define AI_INFERENCE_INTERVAL 1000   // Run every 1 second
#define MOTION_THRESHOLD 30          // Pixel intensity difference
#define CONFIDENCE_THRESHOLD 0.7     // AI confidence cutoff
```

## Adding TensorFlow Lite

To integrate real AI models:

1. Add TFLite library to `platformio.ini`:
   ```ini
   lib_deps = 
       tensorflow/tflite-micro
   ```

2. Replace placeholder in `sense_ai.cpp`:
   ```cpp
   float SenseAI::runInference(camera_fb_t* frame) {
       // Load model and run inference on frame->buf
       // Return actual confidence score
   }
   ```

## Troubleshooting

### Camera Init Failed

```
Camera init failed with error 0x105
```
- Check camera ribbon cable connection
- Verify using XIAO ESP32-S3 **Sense** variant (with camera)
- Ensure PSRAM enabled: `#define BOARD_HAS_PSRAM`

### BLE Not Advertising

```
BLE advertising started
```
- Wait 5-10 seconds after boot
- Use BLE scanner app (e.g., nRF Connect)
- Check device name: `SenseAI_Camera`
- Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`

### WiFi Connection Timeout

```
✗ WiFi connection timeout
```
- Verify SSID and password are correct
- Check 2.4GHz WiFi is enabled (ESP32 doesn't support 5GHz)
- Ensure router is in range

### Twilio Alert Failed

```
Failed to send alert. Error: -1
```
- Verify Twilio credentials in `config.h`
- Check WiFi is connected
- Ensure phone number format: `+[country code][number]`
- Note: Image base64 encoding may exceed Twilio limits for large images

## Development Tips

### Serial Debugging

```bash
# Continuous monitoring
pio device monitor --baud 115200

# With filters
pio device monitor --filter=esp32_exception_decoder
```

### Build Flags

```ini
build_flags = 
    -DCORE_DEBUG_LEVEL=3  # 0=None, 5=Verbose
    -DBOARD_HAS_PSRAM
```

### Memory Usage

```bash
# Check memory after build
pio run --target size
```

## Dependencies

- **ArduinoJson** (7.0.4): JSON parsing for HTTP requests
- **PubSubClient** (2.8): MQTT support (future use)
- **NimBLE-Arduino** (1.4.1): Lightweight BLE stack
- **esp32-camera** (2.0.4): Camera driver for ESP32

## License

MIT

## Resources

- [Seeed XIAO ESP32-S3 Wiki](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- [ESP32 Camera Examples](https://github.com/espressif/esp32-camera)
- [Twilio API Docs](https://www.twilio.com/docs/sms/api)
- [PlatformIO Documentation](https://docs.platformio.org)

## Contributing

Pull requests welcome! Areas for improvement:
- TensorFlow Lite integration for person detection
- Web interface for configuration
- MQTT support for home automation
- Power optimization for battery operation
