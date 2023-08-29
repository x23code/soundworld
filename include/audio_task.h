#ifndef _aduio_task_H
#define _aduio_task_H
#include "Arduino.h"
#include "Audio.h"

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

extern Audio audio;
extern TaskHandle_t audioTaskHandle;
extern String m_list[30];
extern uint8_t m_count;
extern uint8_t m_index;
void audiotask(void *pvParameters);
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void musiclist(fs::FS &fs, const char *dirname, String list[]);
void playSD(String filename);
void playStop();
void playNow(uint8_t index);
void playNext();
void playPre();
#endif
