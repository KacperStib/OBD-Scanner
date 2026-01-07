#include "screen.h"

TFT_eSPI tft = TFT_eSPI();

char text_buf[50];

// wymiary ekranu
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
// bufory
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

// odswiezanie ekranu
void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp_drv );
}

// odczyt infomracji o dotyku ekranu
void my_touchpad_read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        // ustaw koordynaty dotyku
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

// inicjalizacja bilbioteki LVGL
void init_lvgl(){

    lv_init();
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    // inicjalizacja wyswietlacza
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    // inicjalizacja dotyku
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );
    
    // inicjalizacja UI ze squareLineStudio
    ui_init();
}

// inicjalizacja wyswietlacza biblioteki eTFT
void init_tft(){
  pinMode(14, OUTPUT);
  digitalWrite(14,1);
  tft.begin();
  // orientacja ekranu
  tft.setRotation(1);
  // dane kalibracyjne dla wyswietlacza 480x320
  uint16_t calData[5] = {406, 3326, 487, 3169, 1}; //{ 275, 3620, 264, 3532, 1 };
  tft.setTouch( calData );
}

void refresh_parameters(){
    sprintf(text_buf, "%d km/h", speed);
    lv_label_set_text(ui_Speed, text_buf);

    sprintf(text_buf, "%.2f l/100km", fuel_consumption);
    lv_label_set_text(ui_Consump, text_buf);

    sprintf(text_buf, "%.2f l/100km", fuel_consumption_avg);
    lv_label_set_text(ui_ConsumpAvg, text_buf);

    sprintf(text_buf, "%.2f km", distance);
    lv_label_set_text(ui_Distance, text_buf);

    sprintf(text_buf, "%.2f l", fuel);
    lv_label_set_text(ui_Fuel, text_buf);
}