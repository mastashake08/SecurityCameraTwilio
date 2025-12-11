#include "sense_ai.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <base64.h>

SenseAI::SenseAI() : previousFrame(nullptr), lastInferenceTime(0), motionDetected(false) {}

SenseAI::~SenseAI() {
    if (previousFrame) {
        esp_camera_fb_return(previousFrame);
    }
}

void SenseAI::begin() {
    Serial.println("Initializing camera...");
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_QVGA;  // 320x240
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return;
    }
    
    // Get sensor settings
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 0);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 0);     // -2 to 2
        s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect)
        s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
        s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
        s->set_wb_mode(s, 0);        // 0 to 4
        s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
        s->set_aec2(s, 0);           // 0 = disable , 1 = enable
        s->set_ae_level(s, 0);       // -2 to 2
        s->set_aec_value(s, 300);    // 0 to 1200
        s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
        s->set_agc_gain(s, 0);       // 0 to 30
        s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
        s->set_bpc(s, 0);            // 0 = disable , 1 = enable
        s->set_wpc(s, 1);            // 0 = disable , 1 = enable
        s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
        s->set_lenc(s, 1);           // 0 = disable , 1 = enable
        s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
        s->set_vflip(s, 0);          // 0 = disable , 1 = enable
        s->set_dcw(s, 1);            // 0 = disable , 1 = enable
        s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
    }
    
    Serial.println("Camera initialized successfully");
}

bool SenseAI::detectMotion(camera_fb_t* currentFrame) {
    if (!previousFrame || previousFrame->len != currentFrame->len) {
        return false;
    }
    
    uint32_t diffCount = 0;
    uint32_t totalPixels = currentFrame->len;
    
    // Simple pixel difference algorithm
    for (uint32_t i = 0; i < totalPixels; i++) {
        int diff = abs(currentFrame->buf[i] - previousFrame->buf[i]);
        if (diff > MOTION_THRESHOLD) {
            diffCount++;
        }
    }
    
    float changePercentage = (float)diffCount / totalPixels * 100.0;
    Serial.printf("Motion detection: %.2f%% pixels changed\n", changePercentage);
    
    return changePercentage > 5.0;  // 5% change threshold
}

float SenseAI::runInference(camera_fb_t* frame) {
    // Placeholder for AI inference
    // TODO: Integrate TensorFlow Lite or other AI model
    // For now, return confidence based on motion detection
    Serial.println("Running AI inference...");
    return motionDetected ? 0.85 : 0.1;
}

bool SenseAI::processFrame() {
    if (millis() - lastInferenceTime < AI_INFERENCE_INTERVAL) {
        return false;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return false;
    }
    
    // Detect motion
    motionDetected = detectMotion(fb);
    
    // Run AI inference
    float confidence = runInference(fb);
    
    // Store current frame as previous
    if (previousFrame) {
        esp_camera_fb_return(previousFrame);
    }
    previousFrame = fb;
    
    lastInferenceTime = millis();
    
    // Trigger alert if motion detected with high confidence
    if (motionDetected && confidence > CONFIDENCE_THRESHOLD) {
        Serial.printf("⚠️ Event detected! Confidence: %.2f\n", confidence);
        sendAlert();
        return true;
    }
    
    return false;
}

bool SenseAI::hasMotion() {
    return motionDetected;
}

void SenseAI::sendAlert() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, cannot send alert");
        return;
    }
    
    String endpoint = String(ALERT_ENDPOINT);
    if (endpoint.isEmpty()) {
        Serial.println("Alert endpoint not configured");
        return;
    }
    
    // Get current frame for alert
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Failed to capture frame for alert");
        return;
    }
    
    HTTPClient http;
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    
    // Add authentication if configured
    String authToken = String(ALERT_AUTH_TOKEN);
    if (!authToken.isEmpty()) {
        http.addHeader("Authorization", "Bearer " + authToken);
    }
    
    // Encode image to base64
    String imageBase64 = base64::encode(fb->buf, fb->len);
    
    // Create JSON payload
    String payload = "{";
    payload += "\"timestamp\":" + String(millis()) + ",";
    payload += "\"device\":\"" + String(DEVICE_NAME) + "\",";
    payload += "\"event\":\"motion_detected\",";
    payload += "\"confidence\":0.85,";
    payload += "\"image\":\"data:image/jpeg;base64," + imageBase64 + "\"";
    payload += "}";
    
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) {
        Serial.printf("Alert sent successfully. Response: %d\n", httpResponseCode);
        String response = http.getString();
        Serial.printf("Server response: %s\n", response.c_str());
    } else {
        Serial.printf("Failed to send alert. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    
    http.end();
    esp_camera_fb_return(fb);
}
