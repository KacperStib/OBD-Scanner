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

void obd_task(){
	CAN_init();
	for(;;){
		CAN_read();
		// Delay
		vTaskDelay(pdMS_TO_TICKS(1000)); 
	}
	
}

void app_main(void)
{	
	ble_init();
 	// Read obd2 - every 1 second
 	xTaskCreatePinnedToCore(obd_task, "obd", 4096, NULL, 1, NULL, 0);
	
}