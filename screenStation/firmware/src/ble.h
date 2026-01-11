#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEAdvertisedDevice.h>

// Server definitions
#define TARGET_NAME "BLE-Server"
#define SERVICE_UUID        "0BD2" // UUID servis
#define CHARACTERISTIC_UUID "C0DE" // UUID char to read

extern BLEAdvertisedDevice* myDevice;
extern BLEClient* pClient;
extern BLERemoteCharacteristic* pRemoteCharacteristic;

extern bool doConnect;
extern bool connected;
extern std::string val;
extern uint8_t ble_buf[5];

void ble_init();
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
bool connectToServer();
void ble_check_connection();

// Callback classes
// Client callback
class MyClientCallback : public BLEClientCallbacks {
public:
    void onConnect(BLEClient* pclient) override {
        Serial.println("Connected to server");
    }

    void onDisconnect(BLEClient* pclient) override {
        connected = false;
        Serial.println("Disconnected from server");
        doConnect = true; // spróbuj ponownie
    }
};

// Scaner callback
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
public: 
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        Serial.print("Found device: ");
        Serial.println(advertisedDevice.getName().c_str());

        if (advertisedDevice.getName() == TARGET_NAME) {
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
        }
    }
};