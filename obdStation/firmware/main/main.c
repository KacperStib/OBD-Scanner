/*
Dummy CAN BUS Device
ESP32C3 and MCP2515 based
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "sdkconfig.h"
#include "esp_log.h"

#include "obd.h"
#include "ble.h"

//static const char TAG[] = "main";
uint8_t can_buf[8] = {0};

void obd_task(){
	CAN_init();
	for(;;){
		CAN_read(can_buf);
		// Delay
		vTaskDelay(pdMS_TO_TICKS(1000)); 
	}
}

void ble_task(){
	ble_init();
	for(;;){
		for(int i = 0 ; i < 8 ; i++){
			ble_buf[i] = can_buf[i];
		}
	vTaskDelay(pdMS_TO_TICKS(1000)); 
	}
}

void app_main(void)
{	
	
 	// Read obd2 - every 1 second
 	xTaskCreatePinnedToCore(obd_task, "obd", 4096, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(ble_task, "ble", 4096, NULL, 2, NULL, 0);
}