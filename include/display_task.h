#ifndef _display_task_H
#define _display_task_H
#include "TFT_eSPI.h"
#include <U8g2_for_TFT_eSPI.h>
#include "key.h"

#define FONT u8g2_font_wqy16_t_gb2312b
#define page_max 6
#define home_page 0x00
#define websetting_page 0x01
#define systemtime_page 0x02
#define localplayer_page  0x03
#define btplayer_page 0x04
#define weather_page 0x05

#define cmd_log 0x01
#define cmd_key 0x02
#define cmd_song1 0x03
#define cmd_song2 0x04
#define cmd_weather 0x05
#define cmd_web  0x06 
#define cmd_wifi 0x07
extern String strtext1;
extern String strtext2;
extern uint8_t key_status;
extern uint8_t select_num;
extern uint8_t page_index;
extern uint8_t last_pageindex;
extern TFT_eSPI tft;
extern U8g2_for_TFT_eSPI u8f; 
extern TaskHandle_t displayTaskHandle;
extern struct displayMsg
{
    uint8_t cmd;
    String  info;
}dmsg;
extern QueueHandle_t dmsg_queue;
void displayTask(void *pvParameters);
#endif
