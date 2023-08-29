#include "wifimanage.h"
#include "SPIFFS.h"
#include "SD.h"
#include "display_task.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
extern SDFS SD;
extern SPIFFSFS SPIFFS;
extern WiFiClass WiFi;
extern QueueHandle_t dmsg_queue;
extern displayMsg dmsg;
String text;
void do_wificonnect()
{
    String essid = "Xiaomi_E436";
    String password = "hsmy77777";
    File ff;
    if (SPIFFS.exists("/wifirecord.txt"))
    {
        ff = SPIFFS.open("/wifirecord.txt", "r");
        essid = ff.readStringUntil(';');
        password = ff.readStringUntil(';');
        ff.close();
    }
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(essid, password);
    for (int i = 0; i < 10; i++)
    {
        if (WiFi.status() == WL_CONNECTED)
            break;
        else
        {
            delay(500);
            Serial.print(".");
        }
    }
    if (WiFi.isConnected())
    { // 更新网络时间
        configTime(3600 * 8, 0, "ntp1.aliyun.com", "ntp2.aliyun.com", "ntp3.aliyun.com");
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            File tf;
            tf = SPIFFS.open("/timerecord.txt", "w");
            tf.println(&timeinfo, "%F %T %A");
            tf.close();
        }
        dmsg.cmd = cmd_wifi;
        dmsg.info = "wifi连接成功\n网络名称:" + essid;
        xQueueSend(dmsg_queue, &dmsg, 10);
    }
    else
    {
        dmsg.cmd = cmd_wifi;
        dmsg.info = "wifi连接失败";
        xQueueSend(dmsg_queue, &dmsg, 10);
    }
}

bool weathertext()
{

    String url = "https://www.yiketianqi.com/free/day?appid=16571189&appsecret=oFE3cX0D&unescape=1&city=东莞";
    // String city = "城市名";
    // String appid= "16571189";
    // String appsecret="oFE3cX0D";

    // 创建 HTTPClient 对象
    HTTPClient http;

    // 发送GET请求
    http.begin(url);

    int httpCode = http.GET();

    // 获取响应状态码
    Serial.printf("HTTP 状态码: %d", httpCode);
    if (httpCode < 0)
    {
        return false;
    }
    // 获取响应正文
    String response = http.getString();
    http.end();

    // 创建 DynamicJsonDocument 对象
    DynamicJsonDocument doc(1024);

    // 解析 JSON 数据
    deserializeJson(doc, response);

    // 从解析后的 JSON 文档中获取值

    String date = doc["date"].as<String>();
    String week = doc["week"].as<String>();
    String wea = doc["wea"].as<String>();
    String tem = doc["tem"].as<String>();
    String tem_day = doc["tem_day"].as<String>();
    String tem_night = doc["tem_night"].as<String>();
    String win = doc["win"].as<String>();
    String win_speed = doc["win_speed"].as<String>();
    String air = doc["air"].as<String>();
    text = "早上好，现在播放天气预报，今天" + date + "," + week + "," + "东莞," + wea + ",";
    text = text + "温度" + tem + "摄氏度," + "白天" + tem_day + "摄氏度," + "晚上" + tem_night + "摄氏度,";
    text = text + win + "," + "风速" + win_speed + "," + "空气质量" + air;
    Serial.println(text);
    return true;
}

bool weathervoice()
{
    String speechtext = text;
    File fweather;
    fweather = SD.open("/weather.wav", "w");

    Serial.println("合成天气音频");
    HTTPClient http2;
    // configure server and url
    http2.begin("https://tsn.baidu.com/text2audio");
    // start connection and send HTTP header
    http2.addHeader("Accept", "*/*");
    http2.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String token = "24.db0a0c324808717e47acdc742de81525.2592000.1692089951.282335-36248639";
    String data = "tex=" + String(speechtext) + "&tok=" + token + "&cuid=DRBYFG0pnsuC9GvYCpDDGS2xPLlSXLsA";
    data = data + "&ctp=1&lan=zh&spd=5&pit=5&vol=5&per=0&aue=6";
    int httpCode = http2.POST(data);
    Serial.printf("HTTP 状态码: %d", httpCode);
    if (httpCode > 0) // 如果状态码大于0说明请求过程无异常
    {
        if (httpCode == HTTP_CODE_OK) // 请求被服务器正常响应，等同于httpCode == 200
        {
            uint8_t buff[4096] = {0};
            bool pass44 = true;
            int len = http2.getSize(); // 读取响应正文数据字节数，如果返回-1是因为响应头中没有Content-Length属性

            WiFiClient *stream = http2.getStreamPtr(); // 获取响应正文数据流指针

            while (http2.connected() && (len > 0 || len == -1)) // 当前已连接并且有数据可读
            {
                size_t size = stream->available(); // 获取数据流中可用字节数
                if (size)
                {

                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size)); // 读取数据到buff
                    fweather.write(buff, c);
                    if (len > 0)
                    {
                        len -= c;
                    }
                }
            }
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http2.errorToString(httpCode).c_str());
        return false;
    }
    http2.end(); // 结束当前连接

    fweather.close(); // 音频保存完成
    return true;
}