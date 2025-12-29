#include "ble.h"

// --- Definicje zmiennych globalnych ---
BLEAdvertisedDevice* myDevice = nullptr;
BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

bool doConnect = false;
bool connected = false;

// --- Callback do powiadomień ---
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Notify: ");
    for (size_t i = 0; i < length; i++) {
        Serial.printf("%02X ", pData[i]);
    }
    Serial.println();
}

// --- Funkcja łączenia ---
bool connectToServer() {
    if (!myDevice) return false;

    if (!pClient) {
        pClient = BLEDevice::createClient();
        pClient->setClientCallbacks(new MyClientCallback());
    }

    if (!pClient->connect(myDevice)) {
        Serial.println("Failed to connect to server");
        return false;
    }

    BLERemoteService* pService = pClient->getService(SERVICE_UUID);
    if (!pService) {
        Serial.println("Service not found!");
        pClient->disconnect();
        return false;
    }

    pRemoteCharacteristic = pService->getCharacteristic(CHARACTERISTIC_UUID);
    if (!pRemoteCharacteristic) {
        Serial.println("Characteristic not found!");
        pClient->disconnect();
        return false;
    }

    if (pRemoteCharacteristic->canNotify()) {
        pRemoteCharacteristic->registerForNotify(notifyCallback);
    }

    connected = true;
    return true;
}