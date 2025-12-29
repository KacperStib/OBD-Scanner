#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEAdvertisedDevice.h>

// --- Ustawienia serwera ---
#define TARGET_NAME "BLE-Server"
#define SERVICE_UUID        "0BD2" // UUID Twojego serwisu
#define CHARACTERISTIC_UUID "C0DE" // UUID charakterystyki do odczytu

extern BLEAdvertisedDevice* myDevice;
extern BLEClient* pClient;
extern BLERemoteCharacteristic* pRemoteCharacteristic;

extern bool doConnect;
extern bool connected;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
bool connectToServer();

// --- Klasy callbacków ---
// --- Callback klienta ---
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

// --- Callback skanera ---
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