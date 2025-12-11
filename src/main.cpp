#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "config.h"
#include "ble_provisioning.h"
#include "sense_ai.h"

// Global objects
BLEProvisioning bleProvisioning;
SenseAI senseAI;
Preferences preferences;

enum SystemState {
  STATE_BLE_PROVISIONING,
  STATE_WIFI_CONNECTING,
  STATE_RUNNING
};

SystemState currentState = STATE_BLE_PROVISIONING;
unsigned long stateStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== SenseAI Security Camera ===");
  Serial.println("Initializing...");
  
  // Initialize preferences (persistent storage)
  preferences.begin("senseai", false);
  
  // Check if WiFi credentials are already stored
  String storedSSID = preferences.getString("ssid", "");
  String storedPass = preferences.getString("password", "");
  
  if (!storedSSID.isEmpty() && !storedPass.isEmpty()) {
    Serial.println("Found stored WiFi credentials");
    currentState = STATE_WIFI_CONNECTING;
    WiFi.begin(storedSSID.c_str(), storedPass.c_str());
    stateStartTime = millis();
  } else {
    Serial.println("No stored credentials, starting BLE provisioning");
    bleProvisioning.begin();
    stateStartTime = millis();
  }
}

void loop() {
  switch (currentState) {
    
    case STATE_BLE_PROVISIONING: {
      // Check if credentials received via BLE
      if (bleProvisioning.hasCredentials()) {
        String ssid = bleProvisioning.getSSID();
        String pass = bleProvisioning.getPassword();
        
        Serial.printf("Received WiFi credentials: %s\n", ssid.c_str());
        
        // Save credentials
        preferences.putString("ssid", ssid);
        preferences.putString("password", pass);
        
        // Update BLE status
        bleProvisioning.updateStatus("Connecting to WiFi...");
        
        // Connect to WiFi
        WiFi.begin(ssid.c_str(), pass.c_str());
        currentState = STATE_WIFI_CONNECTING;
        stateStartTime = millis();
      }
      
      // Timeout after 5 minutes
      if (millis() - stateStartTime > BLE_PROVISIONING_TIMEOUT) {
        Serial.println("BLE provisioning timeout");
        bleProvisioning.updateStatus("Timeout - Restarting");
        delay(2000);
        ESP.restart();
      }
      break;
    }
    
    case STATE_WIFI_CONNECTING: {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi connected!");
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
        
        // Update BLE status if still running
        if (bleProvisioning.isConnected()) {
          bleProvisioning.updateStatus("WiFi Connected!");
          delay(2000);
          bleProvisioning.stop();
        }
        
        // Initialize camera and AI
        senseAI.begin();
        
        currentState = STATE_RUNNING;
        Serial.println("=== System Ready ===");
        
      } else if (millis() - stateStartTime > WIFI_CONNECT_TIMEOUT) {
        Serial.println("✗ WiFi connection timeout");
        
        // Update BLE status if still running
        if (bleProvisioning.isConnected()) {
          bleProvisioning.updateStatus("WiFi Failed - Retry");
        }
        
        // Clear stored credentials and restart
        preferences.remove("ssid");
        preferences.remove("password");
        Serial.println("Restarting...");
        delay(2000);
        ESP.restart();
      }
      break;
    }
    
    case STATE_RUNNING: {
      // Monitor WiFi connection
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, attempting to reconnect...");
        WiFi.reconnect();
        delay(5000);
        
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("Failed to reconnect, restarting...");
          ESP.restart();
        }
      }
      
      // Process camera frames and run AI inference
      if (senseAI.processFrame()) {
        Serial.println("Event processed and alert sent");
      }
      
      // Small delay to prevent watchdog issues
      delay(10);
      break;
    }
  }
}