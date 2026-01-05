#include <Arduino.h>
#include <Bonezegei_DS1307.h>
#include "CompileTime.h"  
#include "screen.h"

#define DS1307_ADRESS 0x68

#define DS1307_EN_REG 0x00
#define DS1307_SET_REG 0x02

#define DS1307_12H_FORMAT 0x40
#define DS1307_24H_FORMAT 0x3F

extern Bonezegei_DS1307 rtc;
extern char buf[50];
extern uint8_t lastMin;
extern bool i2cFlag;

void setRTC();
void printTime();