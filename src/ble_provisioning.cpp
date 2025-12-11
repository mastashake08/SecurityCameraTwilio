#include "ble_provisioning.h"
#include "config.h"

// Callback class for BLE server events
class ServerCallbacks : public NimBLEServerCallbacks {
    BLEProvisioning* provisioning;
public:
    ServerCallbacks(BLEProvisioning* prov) : provisioning(prov) {}
    
    void onConnect(NimBLEServer* pServer) {
        Serial.println("BLE Client connected");
    }
    
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("BLE Client disconnected");
        NimBLEDevice::startAdvertising();
    }
};

// Callback class for characteristic writes
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    BLEProvisioning* provisioning;
    String* targetString;
    
public:
    CharacteristicCallbacks(BLEProvisioning* prov, String* target) 
        : provisioning(prov), targetString(target) {}
    
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        *targetString = pCharacteristic->getValue().c_str();
        Serial.printf("Received: %s\n", targetString->c_str());
    }
};

BLEProvisioning::BLEProvisioning() 
    : deviceConnected(false), credentialsReceived(false) {}

void BLEProvisioning::begin() {
    Serial.println("Starting BLE provisioning...");
    
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(this));
    
    NimBLEService* pService = pServer->createService(SERVICE_UUID);
    
    // SSID Characteristic
    pSSIDChar = pService->createCharacteristic(
        WIFI_SSID_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    pSSIDChar->setCallbacks(new CharacteristicCallbacks(this, &ssid));
    
    // Password Characteristic
    pPassChar = pService->createCharacteristic(
        WIFI_PASS_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    pPassChar->setCallbacks(new CharacteristicCallbacks(this, &password));
    
    // Status Characteristic (read-only)
    pStatusChar = pService->createCharacteristic(
        STATUS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    pStatusChar->setValue("Waiting for credentials");
    
    pService->start();
    
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    
    Serial.println("BLE advertising started");
}

void BLEProvisioning::stop() {
    NimBLEDevice::deinit(true);
    Serial.println("BLE stopped");
}

bool BLEProvisioning::isConnected() {
    return pServer->getConnectedCount() > 0;
}

bool BLEProvisioning::hasCredentials() {
    credentialsReceived = !ssid.isEmpty() && !password.isEmpty();
    return credentialsReceived;
}

String BLEProvisioning::getSSID() {
    return ssid;
}

String BLEProvisioning::getPassword() {
    return password;
}

void BLEProvisioning::updateStatus(String status) {
    if (pStatusChar) {
        pStatusChar->setValue(status.c_str());
        pStatusChar->notify();
        Serial.printf("Status updated: %s\n", status.c_str());
    }
}
