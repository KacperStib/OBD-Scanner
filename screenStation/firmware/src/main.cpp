#include "ble.h"
#include "screen.h"
#include "clock.h"

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

void lvglAndCmdsTask(void *pvParameters){
  for(;;){
    lv_timer_handler();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  } 
}

void bleTask(void *pvParameters){
  for(;;){
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
    
    vTaskDelay(2000 / portTICK_PERIOD_MS); // odczyt co 2 sekundy
  }
}

void timeTask(void *pvParameters){
    setRTC();
    for(;;){
        printTime();
         vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
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

    // inicjalizacja wyswietlacza
    init_tft();
    init_lvgl();

    // Taski
    // lvgl task - 20 ms
    xTaskCreate(lvglAndCmdsTask, "lvglAndCmdsTask", 4096, NULL, 1, NULL);
    // ble task - 2 s
    xTaskCreate(bleTask, "bleTask", 4096, NULL, 2, NULL);
    // time update task - 1 s
    xTaskCreate(timeTask, "timeTask", 4096, NULL, 3, NULL);
}

// --- Loop ---
void loop() {
}