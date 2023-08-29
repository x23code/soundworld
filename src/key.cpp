#include "key.h"
uint8_t key_status = 0x00;
volatile uint8_t irqflag = 0;
volatile uint8_t pressed = 0;
TickType_t tick_count = 0;
TickType_t tick_gap = 0;
void key_enter_irq()
{
    irqflag = 1;
    pressed = key_enter_press;
}
void key_up_irq()
{
    irqflag = 1;
    pressed = key_up_press;
}
void key_down_irq()
{
    irqflag = 1;
    pressed = key_down_press;
}
void key_init()
{
    pinMode(key_enter, INPUT_PULLUP);
    pinMode(key_up, INPUT_PULLUP);
    pinMode(key_down, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(key_enter), key_enter_irq, FALLING);
    attachInterrupt(digitalPinToInterrupt(key_up), key_up_irq, FALLING);
    attachInterrupt(digitalPinToInterrupt(key_down), key_down_irq, FALLING);
}
uint8_t key_scan()
{
    if (not irqflag) // 没有按键按下
        return 0;
    if (pressed == key_enter_press)
    {
        delay(10);
        if (digitalRead(key_enter) == 0)
        {
            pressed = 0;
            irqflag = 0;
            Serial.printf("key_enter pressed\n");
            return key_enter_press;
        }
    }
    else if (pressed == key_down_press)
    {
        delay(10);
        if (digitalRead(key_down) == 0)
        {
            pressed = 0;
            irqflag = 0;
            Serial.printf("key_down pressed\n");
            return key_down_press;
        }
    }
    else if (pressed == key_up_press)
    {
        delay(10);
        if (digitalRead(key_up) == 0)
        {
            tick_count = xTaskGetTickCount();
            pressed = key_up_longpress;
        }
    }
    if (pressed == key_up_longpress)
    {
        // key_up已经释放
        if (digitalRead(key_up) == 1)
        {
            delay(10);
            tick_gap = xTaskGetTickCount() - tick_count;
            pressed = 0;
            irqflag = 0;
            Serial.printf("按键按下时长：%d\n", tick_gap);
            Serial.printf("key_up realse\n");
            if (tick_gap < 1500)
            {
                return key_up_press;
            }
            else
            {
                return key_up_longpress;
            }
        }
    }
    return 0;
}