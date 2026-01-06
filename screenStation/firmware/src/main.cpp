#include "ble.h"
#include "screen.h"
#include "clock.h"

void lvglTask(void *pvParameters){

  for(;;){
    lv_timer_handler();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  } 
}

void bleTask(void *pvParameters){

  for(;;){
    ble_check_connection();
    
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

void parametersTask(void *pvParameters){
  for(;;){
    refresh_parameters();
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
  }
}
// --- Setup ---
void setup() {
    Serial.begin(115200);

    // Screen Init
    init_tft();
    init_lvgl();
    
    // BLE Init
    ble_init();

    // Taski
    // Core0 - BLE
    xTaskCreatePinnedToCore(bleTask,  "bleTask",  4096, NULL, 1, NULL, 0);
    
    // Core1 - Logic
    xTaskCreatePinnedToCore(lvglTask, "lvglTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(timeTask, "timeTask", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(parametersTask, "parametersTask", 4096, NULL, 3, NULL, 1);
}

// --- Loop ---
void loop() {
}