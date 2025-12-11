#ifndef CONFIG_H
#define CONFIG_H

// Device Configuration
#define DEVICE_NAME "SenseAI_Camera"
#define BLE_PROVISIONING_TIMEOUT 300000  // 5 minutes

// WiFi Configuration
#define WIFI_CONNECT_TIMEOUT 20000
#define WIFI_RECONNECT_INTERVAL 30000

// Camera Configuration (XIAO ESP32-S3 Sense)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39
#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

// SenseAI Configuration
#define AI_INFERENCE_INTERVAL 1000  // Run inference every 1 second
#define MOTION_THRESHOLD 30         // Pixel difference threshold
#define CONFIDENCE_THRESHOLD 0.7    // AI confidence threshold

// Twilio Configuration (set via BLE provisioning or hardcode)
#define TWILIO_ACCOUNT_SID ""
#define TWILIO_AUTH_TOKEN ""
#define TWILIO_FROM_NUMBER ""
#define TWILIO_TO_NUMBER ""

#endif
