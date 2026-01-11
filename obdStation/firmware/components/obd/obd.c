/*
 * obd.c
 *
 *  Created on: 29 gru 2025
 *       Author: Kacper Stiborski
*/
#include "obd.h"
#include "freertos/projdefs.h"

// Initialize SPI bus
esp_err_t SPI_Init(void)
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
		.clock_speed_hz =4000000, // 4mhz
		.spics_io_num = PIN_NUM_CS,
		.queue_size = 256
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

	return ret;
}

// Initialize CAN
void CAN_init(void)
{
	if (MCP2515_init() == ERROR_OK)
	{
		ESP_LOGI(TAG, "MCP INIT OK");
	}
	else 
	{
		ESP_LOGE(TAG, "MCP INIT NOK");
	}
	if (SPI_Init() != ESP_OK)
		ESP_LOGE(TAG, "SPI INIT ERROR");
	
	// MCP2515 config
	MCP2515_reset();
	vTaskDelay(pdMS_TO_TICKS(10));
	MCP2515_setBitrate(CAN_500KBPS, MCP_8MHZ);
	vTaskDelay(pdMS_TO_TICKS(10));
	
	// MASK0 → filters RXF0, RXF1 ext flag false
    MCP2515_setFilterMask(MASK0, false, 0x7F0);  // maska 11-bitowa standard ID
    MCP2515_setFilter(RXF0, false, 0x7DF);       // filter 0 → functional request
    MCP2515_setFilter(RXF1, false, 0x7E1);       // filter 1 → ECU response (np 0x7E8 w decimal = 2016 → 0x7E0 + 8 = 0x7E8)

    // MASK1 → filters RXF2-RXF5
    MCP2515_setFilterMask(MASK1, false, 0x7F0);
    MCP2515_setFilter(RXF2, false, 0x7DF);
    MCP2515_setFilter(RXF3, false, 0x7E1);
    MCP2515_setFilter(RXF4, false, 0x7DF);
    MCP2515_setFilter(RXF5, false, 0x7E1);
    
    MCP2515_setNormalMode();
    
    // Interrupt pin
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << PIN_NUM_INTERRUPT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

// Simple read from CANbus to buf 
void CAN_read(uint8_t *buf)
{
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

// Simple write to CANbus 2 byte value
void CAN_write(uint16_t val)
{
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

// Write to OBD II interface, specify MODE and PID
void OBD_write(uint8_t mode, uint8_t pid)
{
    CAN_FRAME_t frame[1];

    frame[0]->can_id  = 0x7DF;   // OBD2 broadcast
    frame[0]->can_dlc = 8;		 // OBD2 frame length always 8

    frame[0]->data[0] = 0x02;    // bytes
    frame[0]->data[1] = mode;    // mode
    frame[0]->data[2] = pid;     // pid
    frame[0]->data[3] = 0x55;	 // 4 - 8 padding
    frame[0]->data[4] = 0x55;
    frame[0]->data[5] = 0x55;
    frame[0]->data[6] = 0x55;
    frame[0]->data[7] = 0x55;

    if (MCP2515_sendMessageAfterCtrlCheck(frame[0]) != ERROR_OK)
        ESP_LOGE(TAG, "OBD send failed");
}

// Read from OBD II interface to buf (only ECU msgs)
bool OBD_read(uint8_t *buf)
{	
    CAN_FRAME_t frame[1];
	
	// Read only if interrupt pin is low
	if(!gpio_get_level(PIN_NUM_INTERRUPT))
	{	
		uint8_t irq = MCP2515_getInterrupts();
		// Read from register
		if (irq & CANINTF_RX0IF)
		{
			if (MCP2515_readMessage(RXB0, frame[0]) != ERROR_OK)
	    		return false;
	    }
	   	else if (irq & CANINTF_RX1IF)
	   	{
			if (MCP2515_readMessage(RXB1, frame[0]) != ERROR_OK)
	    		return false;
	    }
	    
	    // Debug info
		/*printf("CAN ID: 0x%08lX\n", frame[0]->can_id);
		printf("DLC: %d\n", frame[0]->can_dlc);
		printf("Data: ");
		for (int i = 0; i < frame[0]->can_dlc; i++) 
			printf("%02X ", frame[0]->data[i]);
		printf("\n");*/
		
		// Check if frame is from ECU (0x7E8)
	    if (frame[0]->can_id != 0x7E8)
	        return false;
		
		// Copy frame data to buf
	    //memcpy(buf, frame[0]->data, 8);
	    for (int i = 0; i < frame[0]->can_dlc; i++) {
			buf[i] = frame[0]->data[i];
		}
	    return true;
	}
	return false;
}

bool OBD_read_with_check(uint8_t *buf, uint8_t pid)
{	
	// Timeout
	uint32_t start = xTaskGetTickCount();
	
	// Look for frames for 200ms
	while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(200)){
		// Read to buf
		if (!OBD_read(buf))
		{
			continue;
		}
		
		// Check if msg is mode 41 and about PID
		if (buf[1] == 0x41 || buf[2] == pid)
		{
			ESP_LOGE(TAG, "Found PID!");
			for (int i = 0; i < 8; i++) 
				printf("%02X ", buf[i]);
			printf("\n");
	        return true;
	    }
    }
    return false;
}

// Display supported pids
void OBD_supported_pids() 
{	
	// Ask ECU about PIDs
	OBD_write(CURRENT_DATA, PIDS_SUPPORT);
	//vTaskDelay(pdMS_TO_TICKS(50));
	
	// Read supported PIDs to buf
	uint8_t buf[8];
	if (!OBD_read_with_check(buf, PIDS_SUPPORT))
	{
		ESP_LOGE(TAG, "Timeout no header");
		return;
	}
	    
    // Bitmap of supported PIDs
	uint32_t bitmap =
        (buf[3] << 24) |
        (buf[4] << 16) |
        (buf[5] << 8)  |
        (buf[6]);

    //ESP_LOGI(TAG, "PID bitmap: 0x%08X", bitmap);
	
	// Display supported PIDs
    for (uint8_t pid = 1; pid <= 32; pid++) {
        if (bitmap & (1UL << (32 - pid))) {
            ESP_LOGI(TAG, "PID 0x%02X supported", pid);
        }
    }
}

// Read vehilce velocity
uint8_t OBD_velocity()
{	
	// Ask ECU about vehicle speed
	OBD_write(CURRENT_DATA, VEHICLE_SPEED);
	//vTaskDelay(pdMS_TO_TICKS(50));
	
	// Read data to buf
	uint8_t buf[8];
	if (!OBD_read_with_check(buf, VEHICLE_SPEED))
	{
		ESP_LOGE(TAG, "Timeout no header");
		return 0;
	}
	
	// Read velocity
	uint8_t vel = buf[3];
	
	return vel;
}

// Read engine RPM
uint16_t OBD_RPM()
{
	// Ask ECU about RPM
	OBD_write(CURRENT_DATA, ENGINE_RPM);
	//vTaskDelay(pdMS_TO_TICKS(50));
	
	// Read data to buf
	uint8_t buf[8];
	if (!OBD_read_with_check(buf, ENGINE_RPM))
	{
		ESP_LOGE(TAG, "Timeout no header");
		return 0;
	}
	
	// Read RPM
	uint16_t RPM = (buf[3] * 256 + buf[4]) / 4;
	
	return RPM;
}

// Check if MAF is supported directly
bool OBD_is_MAF_supported()
{
    uint8_t buf[8];

    // Ask for supported PIDs
    OBD_write(CURRENT_DATA, PIDS_SUPPORT);
    //vTaskDelay(pdMS_TO_TICKS(50));
	
	// Read supported PIDs
    if (!OBD_read_with_check(buf, PIDS_SUPPORT))
    {
        ESP_LOGE(TAG, "Timeout no header");
        return false;
    }

    // Supported PIDs bitmap
    uint32_t bitmap =
        (buf[3] << 24) |
        (buf[4] << 16) |
        (buf[5] << 8)  |
        (buf[6]);

    // PID 0x10 = bit 16, check whether MAF is supported
    if (bitmap & (1UL << (32 - 16))) {
        ESP_LOGI(TAG, "MAF PID 0x10 supported");
        return true;
    }
    else {
        ESP_LOGW(TAG, "MAF PID 0x10 NOT supported");
        return false;
    }
}

// Read MAF
uint16_t OBD_MAF()
{	
	uint16_t MAF = 0;
	
	// Standard procedure read from MAF register
	if (OBD_is_MAF_supported())
	{
		// Ask ECU about MAF
		OBD_write(CURRENT_DATA, MAF_AIR_FLOW_RATE);
		//vTaskDelay(pdMS_TO_TICKS(50));
		
		// Read data to buf
		uint8_t buf[8];
		if (!OBD_read_with_check(buf, MAF_AIR_FLOW_RATE))
		{
			ESP_LOGE(TAG, "MAF not available");
			return 0;
		}
		
		// Read MAF
		MAF = (buf[3] * 256 + buf[4]) / 100;
	} 
	
	// If MAF register not supported calculate from MAP and IAT
	else
	{
		// Read data to calculate MAF
		float IAT = (float)OBD_IAT() + 273.15f;
		uint8_t MAP = OBD_MAP();
		uint16_t RPM = OBD_RPM();
		
		// Calculate IMAP
		float IMAP = (float)RPM * (float)MAP / IAT;
		
		// engine constants
		const float VE = 75.0f;       // volumetric efficiency [%]
   		const float ED = 1.5f;        // engine volume step
    	const float MM = 28.97f;      // g/mol air
    	const float R = 8.314f;       // R J/(K*mol)
		
		// Calculate MAF
		MAF = (IMAP/120)*(VE/100)*(ED)*(MM)/(R);
	}
	
	return MAF;
}

// Read Intake air temperature
int8_t OBD_IAT()
{
	// Ask ECU about iat
	OBD_write(CURRENT_DATA, INTAKE_AIR_TEMP);
	//vTaskDelay(pdMS_TO_TICKS(50));
	
	// Read data to buf
	uint8_t buf[8];
	if (!OBD_read_with_check(buf, INTAKE_AIR_TEMP))
	{
		ESP_LOGE(TAG, "Timeout no header");
		return 0;
	}
	
	// Read IAT
	int8_t iat = buf[3] - 40;
	
	return iat;
}
// Read Manifold absolute preasure
uint8_t OBD_MAP()
{
	// Ask ECU about map
	OBD_write(CURRENT_DATA, INTAKE_PRESS);
	//vTaskDelay(pdMS_TO_TICKS(50));
	
	// Read data to buf
	uint8_t buf[8];
	if (!OBD_read_with_check(buf, INTAKE_PRESS))
	{
		ESP_LOGE(TAG, "Timeout no header");
		return 0;
	}
	
	// Read MAP
	uint8_t map = buf[3];
	
	return map;
}