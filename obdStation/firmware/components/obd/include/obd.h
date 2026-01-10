/*
 * obd.h
 *
 *  Created on: 29 gru 2025
 *      Author: Kacper Stiborski
 */
#include "mcp2515.h"
#include "esp_log.h"
#include <string.h>
#include "driver/gpio.h"

static const char TAG[] = "main";

// SPI Pins
#  define PIN_NUM_MISO      5
#  define PIN_NUM_MOSI      6
#  define PIN_NUM_CLK       4
#  define PIN_NUM_CS        7
#  define PIN_NUM_INTERRUPT GPIO_NUM_3

// PIDs
#define PIDS_SUPPORT 0x00
#define ENGINE_RPM 0x0C
#define VEHICLE_SPEED 0x0D
#define MAF_AIR_FLOW_RATE 0x10

bool SPI_Init(void);
void CAN_init(void);
void CAN_read(uint8_t *buf);
void CAN_write(uint16_t val);

void OBD_write(uint8_t mode, uint8_t pid);
bool OBD_read(uint8_t *buf);
void OBD_supported_pids();
void OBD_velocity();