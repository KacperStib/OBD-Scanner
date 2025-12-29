#include "ble.h"

// --- Setup ---
void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE Client");

    BLEDevice::init("");
    BLEScan* pScan = BLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pScan->setInterval(1349);
    pScan->setWindow(449);
    pScan->setActiveScan(true);
    pScan->start(0, false); // skanowanie w tle
}

// --- Loop ---
void loop() {
    if (doConnect) {
        if (connectToServer()) {
            Serial.println("Connected and ready to read");
        }
        doConnect = false;
    }

    if (connected && pRemoteCharacteristic && pRemoteCharacteristic->canRead()) {
        std::string val = pRemoteCharacteristic->readValue();
        Serial.print("Characteristic value: ");
        for (auto c : val) Serial.printf("%02X ", (uint8_t)c);
        Serial.println();
    }

    delay(2000); // odczyt co 2 sekundy
}
