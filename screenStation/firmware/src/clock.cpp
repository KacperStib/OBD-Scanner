#include "clock.h"

Bonezegei_DS1307 rtc(0x68);
char buf[50];
uint8_t lastMin = 0;
bool i2cFlag = 0;

// uruchomienie oraz ustawienie RTC
void setRTC(){
  using namespace CompileTime;
  
  // start RTC
  rtc.begin();
  rtc.setFormat(24);        

  // ustawienie RTC z CompileTime
  rtc.setAMPM(1);           //Set AM or PM    0 = AM  1 =PM
  sprintf(buf, "%02d:%02d:%02d", hour, minute, second);
  rtc.setTime(buf);  //Set Time    Hour:Minute:Seconds
  sprintf(buf, "%02d/%02d/%d", 12, 4, 24);
  rtc.setDate(buf);
}

// wyswietlanie czasu 
void printTime(){
  if(!i2cFlag){
  if (rtc.getTime()) {
    // debug
    Serial.printf("Time %02d:%02d:%02d ", rtc.getHour(), rtc.getMinute(), rtc.getSeconds());
    Serial.printf("Date %02d-%02d-%d \n", rtc.getMonth(), rtc.getDay(), (uint16_t) rtc.getYear() + 2000);
    
    // wyswietlanie czasu i daty na ekranie
    if (rtc.getMinute() != lastMin){
      sprintf(buf, "%02d:%02d", rtc.getHour(), rtc.getMinute());
      lv_label_set_text(ui_Time, buf);

      sprintf(buf, "%02d . %02d . %d", rtc.getDate(), rtc.getMonth(), (uint16_t) rtc.getYear() + 1920);
      lv_label_set_text(ui_Date, buf);
    }
  }
  }
}