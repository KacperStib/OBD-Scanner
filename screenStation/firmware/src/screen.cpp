#include "screen.h"

TFT_eSPI tft = TFT_eSPI();

void tft_init(){
    tft.init();
    tft.setRotation(2);
    pinMode(14, OUTPUT);
    digitalWrite(14,1);
    tft.fillScreen(TFT_GREY);
}