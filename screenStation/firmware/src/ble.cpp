#include "ble.h"

// --- Definicje zmiennych globalnych ---
BLEAdvertisedDevice* myDevice = nullptr;
BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

bool doConnect = false;
bool connected = false;
std::string val;

void ble_init(){
    Serial.println("Starting BLE Client");
    BLEDevice::init("");
    BLEScan* pScan = BLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pScan->setInterval(1349);
    pScan->setWindow(449);
    pScan->setActiveScan(true);
    pScan->start(0, false); // skanowanie w tle
}

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

String toHex(const std::string& s) {
    String out;
    for (uint8_t b : s) {
        if (b < 16) out += "0";
        out += String(b, HEX);
        out += " ";
    }
    return out;
}

void ble_check_connection(){
    if (doConnect) {
        if (connectToServer()) {
            Serial.println("Connected and ready to read");
        }
        doConnect = false;
    }

    if (connected && pRemoteCharacteristic && pRemoteCharacteristic->canRead()) {
        val = pRemoteCharacteristic->readValue();
        Serial.print("Characteristic value: ");
        for (auto c : val) Serial.printf("%02X ", (uint8_t)c);
        Serial.println();
    }
}