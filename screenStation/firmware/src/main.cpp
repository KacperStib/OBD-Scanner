#include "ble.h"
#include "screen.h"
#include "clock.h"
#include "parameters.h"

void lvglTask(void *pvParameters){

  for(;;){
    lv_timer_handler();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  } 
}

void bleTask(void *pvParameters){

  for(;;){
    ble_check_connection();
    
    vTaskDelay(2000 / portTICK_PERIOD_MS); 
  }
}

void timeTask(void *pvParameters){

    setRTC();
    for(;;){
      printTime();
      vTaskDelay(30000 / portTICK_PERIOD_MS); 
    }
}

void parametersTask(void *pvParameters){
  for(;;){
    update_parameters();
    refresh_parameters();
    vTaskDelay(dt / portTICK_PERIOD_MS); 
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
    xTaskCreate(bleTask,  "bleTask",  4096, NULL, 1, NULL);
    
    // Core1 - Logic
    xTaskCreate(lvglTask, "lvglTask", 4096, NULL, 2, NULL);
    xTaskCreate(timeTask, "timeTask", 4096, NULL, 3, NULL);
    xTaskCreate(parametersTask, "parametersTask", 4096, NULL, 4, NULL);
}

// --- Loop ---
void loop() {
}