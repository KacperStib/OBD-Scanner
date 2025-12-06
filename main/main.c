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

#include "mcp2515.h"

// SPI PINS - dummy
#  define PIN_NUM_MISO      5
#  define PIN_NUM_MOSI      6
#  define PIN_NUM_CLK       4
#  define PIN_NUM_CS        7
#  define PIN_NUM_INTERRUPT 3

static const char TAG[] = "main";

// RX and TX frames
CAN_FRAME_t can_frame_rx[1];
CAN_FRAME_t can_frame_tx[1];

uint16_t counter = 1;
bool interrupt = false;

bool SPI_Init(void)
{
	printf("Hello from SPI_Init!\n\r");
	esp_err_t ret;
	//Configuration for the SPI bus
	spi_bus_config_t bus_cfg={
		.miso_io_num=PIN_NUM_MISO,
		.mosi_io_num=PIN_NUM_MOSI,
		.sclk_io_num=PIN_NUM_CLK,
		.quadwp_io_num=-1,
		.quadhd_io_num=-1,
		.max_transfer_sz = 0 // no limit
	};

	// Define MCP2515 SPI device configuration
	spi_device_interface_config_t dev_cfg = {
		.mode = 0, // (0,0)
		.clock_speed_hz = 10000000, // 10mhz
		.spics_io_num = PIN_NUM_CS,
		.queue_size = 128
	};

	// Initialize SPI bus
	ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
	ESP_ERROR_CHECK(ret);
	if (ret == ESP_OK)
		ESP_LOGI(TAG,"SPI INIT OK");

	// Add MCP2515 SPI device to the bus
	ret = spi_bus_add_device(SPI2_HOST, &dev_cfg, &MCP2515_Object->spi);
	ESP_ERROR_CHECK(ret);
	if (ret == ESP_OK)
			ESP_LOGI(TAG, "MCP INIT OK");

	return true;
}

// Initialize CAN
void CAN_init(void)
{
	if (MCP2515_init() == ERROR_OK){
		ESP_LOGI(TAG, "MCP INIT OK");
	}
	else {
		ESP_LOGE(TAG, "MCP INIT NOK");
	}
	SPI_Init();
	MCP2515_reset();
	MCP2515_setBitrate(CAN_125KBPS, MCP_8MHZ);
	MCP2515_setNormalMode();
}

// Write on CANbus
void CAN_write(void){
	// Data
	can_frame_tx[0]->data[0] = (counter >> 8) & 0xFF;  // MSB
	can_frame_tx[0]->data[1] = counter & 0xFF;         // LSB
				
	// Send data
	if(MCP2515_sendMessageAfterCtrlCheck(can_frame_tx[0]) != ERROR_OK) {
		ESP_LOGE(TAG, "Couldn't send message.");
	} 
	else {
		ESP_LOGI(TAG, "Sent CAN value: %d", counter);
	}
				
	// Counter
	counter++;
	if(counter > 65535) counter = 1;
}

// Read from CANbus
void CAN_read(void){
	

	if ((MCP2515_readMessage(RXB0, can_frame_rx[0]) == ERROR_OK) ||
		(MCP2515_readMessage(RXB1, can_frame_rx[0]) == ERROR_OK)) {
			printf("CAN ID: 0x%08lX\n", can_frame_rx[0]->can_id);
			printf("DLC: %d\n", can_frame_rx[0]->can_dlc);
			printf("Data: ");
			for (int i = 0; i < can_frame_rx[0]->can_dlc; i++) {
			        printf("%02X ", can_frame_rx[0]->data[i]);
			}
			printf("\n");
	}
	else {
		printf("No message\n");
	}
	
}

void app_main(void)
{
 	
	// Can initialization
	CAN_init();
	
	// Extended ID, 2 Bytes
	can_frame_tx[0]->can_id = (0x123) | CAN_EFF_FLAG; 
	can_frame_tx[0]->can_dlc = 2;  
		
    while (1) {
			
			//CAN_write();
			CAN_read();
			// Delay
		    vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}
