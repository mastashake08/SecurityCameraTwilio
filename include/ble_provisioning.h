#ifndef BLE_PROVISIONING_H
#define BLE_PROVISIONING_H

#include <NimBLEDevice.h>

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define WIFI_SSID_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define WIFI_PASS_UUID      "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"
#define STATUS_UUID         "9a8ca5ef-e93c-4c9e-a9c9-0c4c8e2d7e8f"

class BLEProvisioning {
private:
    NimBLEServer* pServer;
    NimBLECharacteristic* pSSIDChar;
    NimBLECharacteristic* pPassChar;
    NimBLECharacteristic* pStatusChar;
    bool deviceConnected;
    bool credentialsReceived;
    String ssid;
    String password;

public:
    BLEProvisioning();
    void begin();
    void stop();
    bool isConnected();
    bool hasCredentials();
    String getSSID();
    String getPassword();
    void updateStatus(String status);
};

#endif
