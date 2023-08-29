#ifndef _key_H
#define _key_H
#include <Arduino.h>
#define key_enter 17
#define key_up 33
#define key_down 32

#define key_enter_press 0x01
#define key_up_press 0x02
#define key_up_longpress 0x03
#define key_down_press 0x04
extern uint8_t key_status;

// void key_enter_irq();
// void key_up_irq();
// void key_down_irq();
void key_init();
uint8_t key_scan();
#endif
