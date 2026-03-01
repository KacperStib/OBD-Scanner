# OBD2 Diagnostic Interface with BLE and Display

---

## How to Use the System

The system consists of two devices:

1. **OBD2 Station** – plugged into the vehicle diagnostic port  
2. **Display Station** – receives data via BLE and presents it to the user  

---

## System Startup

### Step 1 – Connect the OBD2 Station

- Plug the device into the vehicle’s OBD2 port.  
- After a few seconds, the green LED will start blinking – this indicates proper CAN bus communication.

If the LED does not blink:

- make sure the ignition is turned ON,  
- verify that the vehicle supports OBD2 over CAN.  

---

### Step 2 – Power the Display Station

- Connect the device via USB power.  
- After startup, it will automatically begin scanning for BLE devices.  
- Once connected, a BLE icon will appear on the screen.  

If the BLE icon does not appear:

- ensure the OBD2 station is powered and connected,  
- check that both devices are within range (a few meters).  

---

## Displayed Data

The screen presents:

- Current vehicle speed  
- Engine RPM  
- Instant fuel consumption  
- Average fuel consumption  
- Traveled distance  
- Fuel used  
- Current date and time  

---

## Reset Function

A **Reset** button is located at the bottom of the screen.

Pressing it resets:

- average fuel consumption  
- traveled distance  
- fuel used  

---

## Measurement Units

- Speed – km/h  
- Engine RPM – revolutions per minute  
- Driving fuel consumption – l/100 km  
- Idle fuel consumption – l/h  

---

## System Limitations

- Not all vehicles provide all PID parameters.  
- Some newer vehicles may restrict access to certain data (e.g., intake temperature).  
- In older vehicles, fuel consumption may be calculated approximately (formula-based).  

---

## Requirements

The vehicle must:

- have an OBD2 connector,  
- support CAN communication (ISO 15765-4),  
- have ignition turned ON.  

---

## Connection Architecture

- OBD2 Station → BLE Peripheral (server)  
- Display Station → BLE Central (client)  

The connection is established automatically after both devices are powered on.

---

## Quick System Flow

Car → OBD2 → CAN → ESP32 → BLE → ESP32 → Display

---

## Additional Information

The system operates in a closed ecosystem and does not require:

- a smartphone,  
- a mobile application,  
- internet access.  

Possible future extensions:

- DTC reading and clearing  
- Extended diagnostic parameters  
- Historical data logging  
