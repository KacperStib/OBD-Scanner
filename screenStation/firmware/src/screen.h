#include <TFT_eSPI.h> 
#include <SPI.h>

#include <lvgl.h>
#include "ui/ui.h"

#define TFT_GREY 0x5AEB 

extern TFT_eSPI tft;

extern char text_buf[50];

void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p );
void my_touchpad_read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data );
void init_lvgl();
void init_tft();
void refresh_parameters();
