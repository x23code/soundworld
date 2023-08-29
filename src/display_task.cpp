#include "display_task.h"
#include "SPIFFS.h"
#include "SD.h"

#include <ESP32Time.h>

extern SDFS SD;
extern SPIFFSFS SPIFFS;
uint8_t select_num = 2;
uint8_t page_index = home_page;
uint8_t last_pageindex = home_page;
TFT_eSPI tft = TFT_eSPI();
U8g2_for_TFT_eSPI u8f;
TaskHandle_t displayTaskHandle = NULL;
QueueHandle_t dmsg_queue = xQueueCreate(8, sizeof(struct displayMsg));

displayMsg dmsg;
struct tm timeinfo;
String strtext1 = ""; // 歌词信息
String strtext2 = "";
void time_set(void)
{
    File file;
    String str_time;
    int year, moon, day, hour, min, sec;
    ESP32Time ttt;
    if (SPIFFS.exists("/timerecord.txt"))
    {
        file = SPIFFS.open("/timerecord.txt", "r"); // 2023-07-01 16:04:24
        str_time = file.readStringUntil('\n');
        Serial.println(str_time);
        Serial.println("timerecord.txt exists");
        file.close();
    }
    else
        str_time = "2023-07-01 16:04:24";
    year = int((str_time[0] - '0') * 1000 + (str_time[1] - '0') * 100 + (str_time[2] - '0') * 10 + (str_time[3] - '0'));
    moon = int((str_time[5] - '0') * 10 + (str_time[6] - '0'));
    day = int((str_time[8] - '0') * 10 + (str_time[9] - '0'));
    hour = int((str_time[11] - '0') * 10 + (str_time[12] - '0'));
    min = int((str_time[14] - '0') * 10 + (str_time[15] - '0'));
    sec = int((str_time[17] - '0') * 10 + (str_time[18] - '0'));
    Serial.printf("%d,%d,%d,%d,%d,%d", year, moon, day, hour, min, sec);
    ttt.setTime(sec, min, hour, day, moon, year);
}
void show_homepage(uint8_t num)
{
    tft.fillScreen(TFT_WHITE);
    u8f.setForegroundColor(TFT_BLACK);
    u8f.setCursor(80, 20);
    u8f.println("主界面");
    tft.setCursor(0, 40);
    tft.drawLine(0, 40, 240, 40, TFT_BLACK);
    u8f.setCursor(50, 70);
    u8f.println("1.配置网络");
    u8f.setCursor(50, 100);
    u8f.setForegroundColor(TFT_RED);
    u8f.println("2.系统时钟");
    u8f.setCursor(50, 130);
    u8f.setForegroundColor(TFT_GREEN);
    u8f.println("3.本地播放");
    u8f.setCursor(50, 160);
    u8f.setForegroundColor(TFT_ORANGE);
    u8f.println("4.蓝牙播放");
    u8f.setCursor(50, 190);
    u8f.setForegroundColor(TFT_BLUE);
    u8f.println("5.天气预报");
    tft.drawRect(10, 50 + (num - 1) * 30, 200, 25, TFT_GREEN);
}

void show_systime(TimerHandle_t xTimer)
{
    TickType_t xLastWakeTime;
    if (page_index == systemtime_page)
    {
        xLastWakeTime = xTaskGetTickCount();
        u8f.setForegroundColor(TFT_BLACK);
        u8f.setCursor(80, 20);
        u8f.println("系统时间");
        tft.setCursor(0, 40);
        tft.drawLine(0, 40, 240, 40, TFT_BLACK);
        tft.setTextColor(TFT_YELLOW);
        tft.setTextFont(4);
        if (getLocalTime(&timeinfo, 300))
        {
            xTaskDelayUntil(&xLastWakeTime, 500);
            if (page_index == systemtime_page)
            {
                tft.fillRect(0, 40, 240, 160, TFT_BLACK);
                tft.setCursor(50, 70);
                tft.print(&timeinfo, "%F");
                tft.setCursor(60, 150);
                tft.print(&timeinfo, "%T");
                tft.setCursor(80, 110);
                tft.print(&timeinfo, "%a");
            }
        }
    }
}
void show_localplayer()
{
    u8f.setForegroundColor(TFT_BLACK);
    u8f.setCursor(80, 20);
    u8f.println("本地播放");
    tft.setCursor(0, 40);
    tft.drawLine(0, 40, 240, 40, TFT_BLACK);
}
void show_btplayer()
{
    u8f.setForegroundColor(TFT_BLACK);
    u8f.setCursor(80, 20);
    u8f.println("蓝牙播放");
    tft.setCursor(0, 40);
    tft.drawLine(0, 40, 240, 40, TFT_BLACK);
}
void show_weather()
{
    u8f.setForegroundColor(TFT_BLACK);
    u8f.setCursor(80, 20);
    u8f.println("天气预报");
    tft.setCursor(0, 40);
    tft.drawLine(0, 40, 240, 40, TFT_BLACK);

    tft.fillRect(0, 50, 240, 80, TFT_WHITE);
    u8f.setForegroundColor(TFT_BLACK);
    u8f.setCursor(10, 70);
    u8f.println("长按key_up播放天气预报");
}
void show_websetting()
{
    u8f.setForegroundColor(TFT_BLACK);
    u8f.setCursor(80, 20);
    u8f.println("配置网络");
    tft.setCursor(0, 40);
    tft.drawLine(0, 40, 240, 40, TFT_BLACK);
    tft.fillRect(0, 50, 240, 80, TFT_WHITE);
    u8f.setForegroundColor(TFT_BLACK);
    u8f.setCursor(10, 70);
    u8f.println("长按key_up使用web配网");
}
void displayTask(void *pvParameters)
{
    tft.init();
    u8f.begin(tft);
    u8f.setFontDirection(0); // left to right (this is default)
    u8f.setForegroundColor(TFT_WHITE);
    u8f.setBackgroundColor(TFT_WHITE);
    u8f.setFontMode(0); // use u8g2 none transparent modes
    u8f.setFont(FONT);
    tft.fillScreen(TFT_BLACK);
    // 显示主界面
    show_homepage(select_num);
    // 设置本地时间
    time_set();
    // 软件定时器，用于刷新屏幕/系统时间页面
    TimerHandle_t lockhandle = xTimerCreate("show_systime", 500, pdTRUE, (void *)0, show_systime);
    xTimerStart(lockhandle, 10);
    displayMsg readbuffer;
    // 等待消息通知
    while (1)
    {
        if (xQueueReceive(dmsg_queue, &readbuffer, portMAX_DELAY) == pdPASS)
        {
            if (readbuffer.cmd == cmd_log)
            {
                tft.fillRect(0, 50, 240, 110, TFT_GREEN);
                u8f.setForegroundColor(TFT_BLACK);
                u8f.setCursor(0, 70);
                u8f.println(readbuffer.info);
                vTaskDelay(2000);
                tft.fillScreen(TFT_WHITE);
            }
            if (last_pageindex != page_index)
            {
                tft.fillScreen(TFT_WHITE);
                last_pageindex = page_index;
            }
        }
        switch (page_index)
        {
        case home_page:
            show_homepage(select_num);
            break;
        case websetting_page:
            show_websetting();
            if (readbuffer.cmd == cmd_wifi)
            {
                tft.fillRect(0, 100, 240, 80, TFT_WHITE);
                u8f.setForegroundColor(TFT_BLACK);
                u8f.setCursor(50, 100);
                u8f.println(readbuffer.info);
            }
            break;
        case systemtime_page:
            break;
        case localplayer_page:
            show_localplayer();
            if (readbuffer.cmd == cmd_key)
            {
                tft.fillRect(0, 50, 240, 80, TFT_WHITE);
                u8f.setForegroundColor(TFT_BLACK);
                u8f.setCursor(10, 70);
                u8f.println(readbuffer.info);
                vTaskDelay(500);
                tft.fillRect(0, 50, 240, 25, TFT_WHITE);
                u8f.setCursor(10, 70);
                u8f.println("正在播放");
            }
            break;
        case btplayer_page:
            show_btplayer();
            if (readbuffer.cmd == cmd_song1)
            {
                tft.fillRect(0, 80, 240, 50, TFT_WHITE);
                u8f.setForegroundColor(TFT_GREEN);
                u8f.setCursor(30, 100);
                u8f.println(strtext1);
            }
            else if (readbuffer.cmd == cmd_song2)
            {
                tft.fillRect(0, 50, 230, 25, TFT_WHITE);
                u8f.setForegroundColor(TFT_BLUE);
                u8f.setCursor(30, 70);
                u8f.println(strtext2);
            }
            break;
        case weather_page:
            show_weather();
            if (readbuffer.cmd == cmd_weather)
            {
                tft.fillRect(0, 50, 240, 80, TFT_WHITE);
                u8f.setForegroundColor(TFT_BLACK);
                u8f.setCursor(10, 70);
                u8f.println(readbuffer.info);
            }
            break;
        default:
            break;
        }
        vTaskDelay(10);
    }
}