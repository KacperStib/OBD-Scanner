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
// 1 byte - velocity, 2 byte - RPM, 2 byte - MAF
uint8_t can_buf[5] = {0};
uint16_t test_RPM = 1800;
uint16_t test_MAF = 50;

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
		ble_buf[0] = can_buf[0] * 256 + can_buf[1];
		ble_buf[1] = (test_RPM >> 8) & 0xFF;
		ble_buf[2] = test_RPM & 0xFF;
		ble_buf[3] = (test_MAF) >> 8 & 0xFF;
		ble_buf[4] = test_MAF & 0xFF;
		// full buf
		/*for(int i = 0 ; i < 8 ; i++){
			ble_buf[i] = can_buf[i];
		}*/
		vTaskDelay(pdMS_TO_TICKS(1000)); 
	}
}

void app_main(void)
{	
	
 	// Read obd2 - every 1 second
 	xTaskCreatePinnedToCore(obd_task, "obd", 4096, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(ble_task, "ble", 4096, NULL, 2, NULL, 0);
}