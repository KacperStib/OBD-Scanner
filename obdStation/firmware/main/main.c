/*
 * main.c
 *
 *  Created on: 29 gru 2025
 *       Author: Kacper Stiborski
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
uint8_t velocity = 0;
uint16_t RPM = 0;
uint16_t MAF = 0;

uint16_t test_RPM = 1800;
uint16_t test_MAF = 50;

bool mozna = true;

void obd_task(){
	// Initialize CANbus
	CAN_init();
	// Wait
	vTaskDelay(pdMS_TO_TICKS(5000)); 
	// Show supported PIDs
	OBD_write(0x01, PIDS_SUPPORT);
	vTaskDelay(pdMS_TO_TICKS(100));
	for(;;){
		// Read data from OBD II interface
		velocity = OBD_velocity();
		vTaskDelay(pdMS_TO_TICKS(100));
		RPM = OBD_RPM();
		vTaskDelay(pdMS_TO_TICKS(100));
		MAF = OBD_MAF();
		vTaskDelay(pdMS_TO_TICKS(100));
		ESP_LOGI(TAG, "Velocity: %d, RPM: %d, MAF: %d", velocity, RPM, MAF);
		// Delay;
		vTaskDelay(pdMS_TO_TICKS(1000)); 
	}
}

void ble_task(){
	// Initialize BLE
	ble_init();
	for(;;){
		ble_buf[0] = velocity;
		ble_buf[1] = (RPM >> 8) & 0xFF;
		ble_buf[2] = RPM & 0xFF;
		ble_buf[3] = (MAF) >> 8 & 0xFF;
		ble_buf[4] = MAF & 0xFF;
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