/*
 * obd.h
 *
 *  Created on: 29 gru 2025
 *      Author: Kacper Stiborski
 */
#include "mcp2515.h"
#include "esp_log.h"

static const char TAG[] = "main";

// SPI Pins
#  define PIN_NUM_MISO      5
#  define PIN_NUM_MOSI      6
#  define PIN_NUM_CLK       4
#  define PIN_NUM_CS        7
#  define PIN_NUM_INTERRUPT 3

bool SPI_Init(void);
void CAN_init(void);
void CAN_read(void);