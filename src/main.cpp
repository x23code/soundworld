#include <Arduino.h>
#include "key.h"
#include "SPI.h"
#include "SD.h"
#include "SPIFFS.h"
#include "Audio.h"
#include "BluetoothA2DPSink.h"
#include "audio_task.h"
#include "display_task.h"
#include "wifimanage.h"

#define pin_led 2
extern SDFS SD;
extern SPIFFSFS SPIFFS;
extern WiFiClass WiFi;
extern TFT_eSPI tft;
extern TaskHandle_t displayTaskHandle;
extern QueueHandle_t dmsg_queue;
extern displayMsg dmsg;
extern Audio audio;
extern TaskHandle_t audioTaskHandle;
extern String strtext1;
extern String strtext2;
BluetoothA2DPSink a2dp_sink;
uint8_t started = 0;

void btcallback(uint8_t id, const uint8_t *text)
{
    Serial.printf("==> AVRC metadata:id 0x%x, %s\n", id, text);
    if (id == 0x1)
    {
        strtext1 = (const char *)text;
        dmsg.cmd = cmd_song1;
        dmsg.info = "song";
        xQueueSend(dmsg_queue, &dmsg, 10);
    }
    else if (id == 0x2)
    {
        strtext2 = (const char *)text;
        dmsg.cmd = cmd_song2;
        dmsg.info = "song";
        xQueueSend(dmsg_queue, &dmsg, 100);
    }
}

void setup()
{
    // put your setup code here, to run once:
    pinMode(pin_led, OUTPUT);
    digitalWrite(pin_led, HIGH);
    key_init();
    Serial.begin(115200);
    SPIFFS.begin(true);
    dmsg.info = "SD卡加载成功";
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI); // 使用VSPI
    SPI.setFrequency(1000000);
    if (!SD.begin(SD_CS))
    {
        dmsg.info = "SD卡加载失败";
    }

    //  显示屏任务
    xTaskCreatePinnedToCore(displayTask, "displayTask", 1024 * 4, NULL, 3, &displayTaskHandle, 1);
    dmsg.cmd = cmd_log;
    xQueueSend(dmsg_queue, &dmsg, 100);

    // 本地播放任务
    xTaskCreatePinnedToCore(audiotask, "audioplay", 1024 * 8, NULL, 2, &audioTaskHandle, 1);
    musiclist(SD, "/music", m_list);
    //  蓝牙歌曲信息回调函数
    a2dp_sink.set_avrc_metadata_attribute_mask(ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM);
    a2dp_sink.set_avrc_metadata_callback(btcallback);
}

// 按键扫描，发送队列消息
void loop()
{
    // put your main code here, to run repeatedly:
    key_status = key_scan();
    if (not key_status)
    {
        return;
    }
    dmsg.cmd = cmd_key;
    dmsg.info = "";
    switch (key_status)
    {
    case key_enter_press:
        dmsg.info = "enter";
        if (page_index == home_page) // enter键：进入页面或者返回主页面
        {
            page_index = select_num; // 选择页面
            // 开启蓝牙播放
            if (page_index == btplayer_page)
            {
                if (audioTaskHandle != NULL)
                    vTaskSuspend(audioTaskHandle);
                if (started == 0)
                {
                    started = 1;
                    a2dp_sink.start("soundworld");
                }
            }
            else if (page_index == localplayer_page)
            {
                // if (audioTaskHandle == NULL)
                //     xTaskCreatePinnedToCore(audiotask, "audioplay", 1024 * 4, NULL, 2, &audioTaskHandle, 1);
                dmsg.info = "正在播放\n\n" + m_list[m_index];
                playNow(m_index);
            }
        }
        else
        { // 退出蓝牙播放
            if (page_index == btplayer_page)
            {
                a2dp_sink.disconnect();
                // btStop();
                vTaskDelay(100);
                if (audioTaskHandle != NULL)
                    vTaskResume(audioTaskHandle);
            }
            else if (page_index == localplayer_page)
            {
                playStop();
            }
            page_index = home_page; // 返回主页面
        }
        break;
    case key_down_press:
        if (page_index == home_page)
        {
            select_num += 1;
            if (select_num == page_max)
                select_num = 1;
            dmsg.info = "down";
        }
        else if (page_index == localplayer_page)
        {
            playNext();
            dmsg.info = "播放下一首\n\n" + m_list[m_index];
        }
        else if (page_index == btplayer_page)
        {
            a2dp_sink.next();
            dmsg.info = "播放下一首\n\n";
        }
        break;
    case key_up_press:
        if (page_index == home_page) // 页面上下选择、歌曲上下播放
        {
            select_num -= 1;
            if (select_num == 0)
                select_num = page_max - 1;
            dmsg.info = "up";
        }
        else if (page_index == localplayer_page)
        {
            playPre();
            dmsg.info = "播放上一首\n\n" + m_list[m_index];
        }
        else if (page_index == btplayer_page)
        {
            a2dp_sink.previous();
            dmsg.info = "播放上一首\n\n";
        }
        break;
    case key_up_longpress:
        if (page_index == websetting_page) // 长按设置网络或者播放天气预报
        {
            //do_wificonnect();
            // dmsg.info = "先连接到热点:ESP32 ,浏览器输入并访问192.168.4.1,填写WiFi信息";
            // dmsg.cmd = cmd_web;
            // xQueueSend(dmsg_queue, &dmsg, 10);
            // dmsg.info = "";
            // 待补充
        }

        else if (page_index == weather_page)
        {
            dmsg.info = "播放天气预报";
            dmsg.cmd = cmd_weather;
            xQueueSend(dmsg_queue, &dmsg, 10);
            dmsg.info = "";
            //do_wificonnect();
            if (not WiFi.isConnected())
            {              
                break;
            }
            if(not weathertext())
            {
                break;
            }
            if(not weathervoice())
            {
                break;
            }
            playSD("/weather.wav");
            page_index = home_page;
            dmsg.cmd=cmd_key;
            dmsg.info="enter";
        }

        break;
    default:
        break;
    }
    key_status = 0x00;
    if (dmsg.info != "")
    {
        xQueueSend(dmsg_queue, &dmsg, 10);
    }
}
