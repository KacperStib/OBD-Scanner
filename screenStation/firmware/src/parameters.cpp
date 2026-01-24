#include "parameters.h"

uint8_t speed = 0;
uint16_t RPM = 0;
uint16_t MAF = 0;

float fuel_lph = 0;
float fuel_consumption = 0;
float fuel_consumption_avg = 0;
float distance = 0;
float fuel = 0;

// Delay beetwen task cycles
uint8_t dt = 100;
float dt_s = float(dt) / 1000.0f;

float eps = 0.01;

void update_parameters()
{
    // Update from ble buffor
    speed = ble_buf[0];
    RPM = (ble_buf[1] << 8) | ble_buf[2];
    MAF = (ble_buf[3] << 8) | ble_buf[4];
    Serial.printf("Speed: %d, RPM: %d, MAF: %d dt: %f\n", speed, RPM, MAF, dt_s);
    
    // Calculate lph consumption even at stop
    MAF = MAF / 2;
    fuel_lph = (MAF * 3600.0) / (14.7 * 832.0); // AFR * FUEL_DENSITIY
    // petrol 745
    // diesel 832
    // Fuel used (even at stop)
    fuel = fuel + fuel_lph * (dt_s / 3600.0);

    // Calculate parameters while driving
    if (speed > 1){
        // Fuel consumption at the moment
        fuel_consumption = (fuel_lph / speed) * 100;
        // Distance
        distance = distance + speed * (dt_s / 3600.0);
        // Average fuel Consumption
        if (distance > eps)
            fuel_consumption_avg = (fuel / distance) * 100;
        // Debug
        Serial.printf("LPH: %f, L100KM: %f, L100KM_AVG: %f, DIST: %f, FUEL: %f\n", fuel_lph, fuel_consumption, fuel_consumption_avg, distance, fuel);
    }

    // Reset average fuel conumsption, distance and fuel used
    if (reset == true)
    {
        fuel_consumption_avg = 0;
        distance = 0;
        fuel = 0;
        reset = false;
    }
}