/*
 * obd.c
 *
 *  Created on: 29 gru 2025
 *       Author: Kacper Stiborski
*/
#include "obd.h"

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
	MCP2515_setBitrate(CAN_500KBPS, MCP_8MHZ);
	MCP2515_setNormalMode();
}

// Read from CANbus
void CAN_read(uint8_t *buf){
	CAN_FRAME_t can_frame_rx[1];

	if ((MCP2515_readMessage(RXB0, can_frame_rx[0]) == ERROR_OK) ||
		(MCP2515_readMessage(RXB1, can_frame_rx[0]) == ERROR_OK)) {
			printf("CAN ID: 0x%08lX\n", can_frame_rx[0]->can_id);
			printf("DLC: %d\n", can_frame_rx[0]->can_dlc);
			printf("Data: ");
			for (int i = 0; i < can_frame_rx[0]->can_dlc; i++) {
			        printf("%02X ", can_frame_rx[0]->data[i]);
			        buf[i] = can_frame_rx[0]->data[i];
			}
			printf("\n");
	}
	else {
		printf("No message\n");
	}
	
}

void CAN_write(uint16_t val){
	CAN_FRAME_t can_frame_tx[1];
	// ID and DLC
	can_frame_tx[0]->can_id = (0x123) | CAN_EFF_FLAG; 
	can_frame_tx[0]->can_dlc = sizeof(val); 
	// Data
	can_frame_tx[0]->data[0] = (val >> 8) & 0xFF;  // MSB
	can_frame_tx[0]->data[1] = val & 0xFF;         // LSB
				
	// Send data
	if(MCP2515_sendMessageAfterCtrlCheck(can_frame_tx[0]) != ERROR_OK) {
		ESP_LOGE(TAG, "Couldn't send message.");
	} 
	else {
		ESP_LOGI(TAG, "Sent CAN value: %d", val);
	}
}

void OBD_write(uint8_t mode, uint8_t pid)
{
    CAN_FRAME_t frame[1];

    frame[0]->can_id  = 0x7DF;   // OBD2 broadcast
    frame[0]->can_dlc = 8;

    frame[0]->data[0] = 0x02;    // ilość bajtów
    frame[0]->data[1] = mode;    // 0x01
    frame[0]->data[2] = pid;     // np. 0x0C
    frame[0]->data[3] = 0x00;
    frame[0]->data[4] = 0x00;
    frame[0]->data[5] = 0x00;
    frame[0]->data[6] = 0x00;
    frame[0]->data[7] = 0x00;

    if (MCP2515_sendMessageAfterCtrlCheck(frame[0]) != ERROR_OK)
        ESP_LOGE(TAG, "OBD send failed");
}

bool OBD_read(uint8_t *buf)
{
    CAN_FRAME_t frame[1];
	
	if (MCP2515_readMessage(RXB0, frame[0]) != ERROR_OK &&
    	MCP2515_readMessage(RXB1, frame[0]) != ERROR_OK)
    	return false;
    	
	printf("CAN ID: 0x%08lX\n", frame[0]->can_id);
	printf("DLC: %d\n", frame[0]->can_dlc);
	printf("Data: ");
			for (int i = 0; i < frame[0]->can_dlc; i++) {
			        printf("%02X ", frame[0]->data[i]);
			        buf[i] = frame[0]->data[i];
			}
	printf("\n");
	
    if (frame[0]->can_id != 0x7E8)
        return false;
	
    //memcpy(buf, frame[0]->data, 8);
    for (int i = 0; i < frame[0]->can_dlc; i++) {
			        printf("%02X ", frame[0]->data[i]);
			        buf[i] = frame[0]->data[i];
	}
    return true;
}

void OBD_supported_pids() 
{
	OBD_write(1, 0);
	vTaskDelay(pdMS_TO_TICKS(50));
	
	uint8_t buf[8];
	if (!OBD_read(buf))
		ESP_LOGE(TAG, "BAD MSG!");
	
	if (buf[1] != 0x41 || buf[2] != 0x00){
		ESP_LOGE(TAG, "BAD HEADER");
        return;
       }
	uint32_t bitmap =
        (buf[3] << 24) |
        (buf[4] << 16) |
        (buf[5] << 8)  |
        (buf[6]);

    //ESP_LOGI(TAG, "PID bitmap: 0x%08X", bitmap);

    for (uint8_t pid = 1; pid <= 32; pid++) {
        if (bitmap & (1UL << (32 - pid))) {
            ESP_LOGI(TAG, "PID 0x%02X supported", pid);
        }
    }
}