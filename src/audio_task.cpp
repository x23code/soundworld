#include "audio_task.h"

// 使用内部DAC
Audio audio(true, I2S_DAC_CHANNEL_RIGHT_EN, I2S_NUM_0);
TaskHandle_t audioTaskHandle = NULL;
extern SDFS SD;
String m_list[30] = {};
uint8_t m_count = 0;
uint8_t m_index = 0;
void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);
  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.print(file.name());
      time_t t = file.getLastWrite();
      struct tm *tmstruct = localtime(&t);
      Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      if (levels)
      {
        listDir(fs, file.path(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.print(file.size());
      time_t t = file.getLastWrite();
      struct tm *tmstruct = localtime(&t);
      Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    file = root.openNextFile();
  }
  root.close();
  file.close();
}
void musiclist(fs::FS &fs, const char *dirname, String list[])
{
  //Serial.printf("List_music directory: %s\n", dirname);
  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
    }
    else
    {
      String tm = file.name();
      tm.toLowerCase();
      if (tm.endsWith(".mp3") || tm.endsWith(".wav"))
      {
        list[m_count] = tm;
        //Serial.println(file.name());
        m_count++;
        if (m_count >= 29)
          break;
      }
    }
    file = root.openNextFile();
  }
  file.close();
  root.close();
  //Serial.printf("音频文件个数%d\n", m_count);
}
void playSD(String filename)
{
 audio.connecttoFS(SD, filename.c_str());
}
void playStop()
{
  audio.stopSong();
  vTaskDelay(10);
}
void playNow(uint8_t index)
{
  String filename = m_list[index];
  filename = "/music/" + filename;
  audio.stopSong();
  delay(3);
  audio.connecttoFS(SD, filename.c_str());
}
void playNext()
{

  if (m_index == m_count)
    m_index = 0;
  else
    m_index = m_index + 1;
  playNow(m_index);
}
void playPre()
{
  if (m_index == 0)
    m_index = m_count;
  else
    m_index--;
  playNow(m_index);
}

//----------------------------------------------------------------------------------------------------------------------
void audiotask(void *pvParameters)
{
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(20); // 0...21
  while (1)
  {
    vTaskDelay(5);
    audio.loop();
  }
}
//----------------------------------------------------------------------------------------------------------------------

// optional
void audio_info(const char *info)
{
  Serial.print("info        ");
  Serial.println(info);
}
void audio_id3data(const char *info)
{ // id3 metadata
  Serial.print("id3data     ");
  Serial.println(info);
}
void audio_eof_mp3(const char *info)
{ // end of file
  Serial.print("eof_mp3     ");
  Serial.println(info);
  //playNext();  //继续播放
}
