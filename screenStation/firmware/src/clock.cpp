#include "clock.h"

MCP7940_Class MCP7940;
char buf[50];
uint8_t lastMin = 0;
bool i2cFlag = 0;

// Initizalzation and configuration of RTC
void setRTC()
{
  using namespace CompileTime;      

  // Initialization of MCP749xx
  while (!MCP7940.begin()) 
  {  
    Serial.println(F("Unable to find MCP7940N. Checking again in 3s."));  
    delay(3000);
  }  
  Serial.println(F("MCP7940N initialized."));
  
  // Check and clear powerfail, also turn oscillator
  if (MCP7940.getPowerFail()) 
  {  
    MCP7940.clearPowerFail();  
  } else 
  {
    while (!MCP7940.deviceStatus()) 
    { 
      Serial.println(F("Oscillator is off, turning it on."));
      bool deviceStatus = MCP7940.deviceStart();  
      if (!deviceStatus) 
      {                       
        Serial.println(F("Oscillator did not start, trying again."));  
        delay(1000);                                                   
      }                
    }      
    
    // If using for first time uncomment this line to adjust time to CompileTime.h library
    //MCP7940.adjust(); 

    // Turn on battery backup mode
    Serial.println(F("Enabling battery backup mode"));
    MCP7940.setBattery(true);     
    if (!MCP7940.getBattery()) 
    {  
      Serial.println(F("Couldn't set Battery Backup, is this a MCP7940N?"));
    }                       
  }  

}

// Check and print time on screen
void printTime()
{
  if(!i2cFlag)
  {
    DateTime now = MCP7940.now();
    // debug
    //Serial.printf("Time %02d:%02d:%02d ", now.hour(), now.minute(), now.second());
    //Serial.printf("Date %02d-%02d-%d \n", now.month(), now.day(), (uint16_t) now.year());
    
    // Only if minute has changed
    if (now.minute() != lastMin)
    {
      sprintf(buf, "%02d:%02d", now.hour(), now.minute());
      lv_label_set_text(ui_Time, buf);

      sprintf(buf, "%02d . %02d . %d", now.day(), now.month(), (uint16_t) now.year());
      lv_label_set_text(ui_Date, buf);

      lastMin = now.minute();
    }
  }
}