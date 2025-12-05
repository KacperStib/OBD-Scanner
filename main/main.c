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

// SPI PINS
#  define PIN_NUM_MISO      13
#  define PIN_NUM_MOSI      12
#  define PIN_NUM_CLK       11
#  define PIN_NUM_CS        10

static const char TAG[] = "main";

CAN_FRAME_t can_frame_rx[1];
uint16_t counter = 1;

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

	// Add MCP2515 SPI device to the bus
	ret = spi_bus_add_device(SPI2_HOST, &dev_cfg, &MCP2515_Object->spi);
	ESP_ERROR_CHECK(ret);

	return true;
}

void CAN_init(void)
{
	MCP2515_init();
	SPI_Init();
	MCP2515_reset();
	MCP2515_setBitrate(CAN_1000KBPS, MCP_8MHZ);
	MCP2515_setNormalMode();
}

void app_main(void)
{
 	
	// Can initialization
	CAN_init();
	
	// Extended ID, 2 Bytes
	can_frame_rx[0]->can_id = (0x123) | CAN_EFF_FLAG; 
	can_frame_rx[0]->can_dlc = 2;  
		
    while (1) {
			// Data
        	can_frame_rx[0]->data[0] = (counter >> 8) & 0xFF;  // MSB
		    can_frame_rx[0]->data[1] = counter & 0xFF;         // LSB
			
			// Send data
		    if(MCP2515_sendMessageAfterCtrlCheck(can_frame_rx[0]) != ERROR_OK) {
		        ESP_LOGE(TAG, "Couldn't send message.");
		    } else {
		        ESP_LOGI(TAG, "Sent CAN value: %d", counter);
		    }
			
			// Counter
		    counter++;
		    if(counter > 65535) counter = 1;
			
			// Delay
		    vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}
