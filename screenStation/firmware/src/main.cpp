#include "ble.h"
#include "screen.h"

std::string val;

String toHex(const std::string& s) {
    String out;
    for (uint8_t b : s) {
        if (b < 16) out += "0";
        out += String(b, HEX);
        out += " ";
    }
    return out;
}

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

    tft_init();
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
        val = pRemoteCharacteristic->readValue();
        Serial.print("Characteristic value: ");
        for (auto c : val) Serial.printf("%02X ", (uint8_t)c);
        Serial.println();
    }

    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
    tft.println(toHex(val));
    
    delay(2000); // odczyt co 2 sekundy
}