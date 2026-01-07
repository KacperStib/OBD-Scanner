#include "ble.h"
#include "ui/ui.h"

extern uint8_t speed;
extern uint16_t RPM;
extern uint16_t MAF;

extern float fuel_consumption;
extern float fuel_consumption_avg;
extern float distance;
extern float fuel;
extern uint8_t dt;
extern float dt_s;

void update_parameters();