#include "main.h"
#include "myTask.h"
#include "SPIFFS.h"
#include "WiFi.h"

TaskHandle_t WIFI_TaskHandle;
TaskHandle_t LCD_TaskHandle;
TaskHandle_t IO_TaskHandle;

float WIFI_task_useage_percent, LCD_task_useage_percent, IO_task_useage_percent, LOOP_task_useage_percent;
uint32_t LOOP_task_runTime = 0;

void setup()
{
  debugSerial.begin(115200);
  debugSerial.print("\r\n\r\n\r\n");
  debugSerial.printf("Model: %s\r\n", ESP.getChipModel());
  debugSerial.printf("SDK version: %s\r\n", ESP.getSdkVersion());
  debugSerial.printf("MAC address: %s\r\n", WiFi.macAddress().c_str());
  debugSerial.printf("Chip revision: %u\r\n", ESP.getChipRevision());
  debugSerial.printf("Cores numbs: %u\r\n", ESP.getChipCores());
  debugSerial.printf("CPU clock: %u MHz\r\n", ESP.getCpuFreqMHz());
  debugSerial.printf("Sytem tick rate: %u Hz\r\n", xPortGetTickRateHz());
  debugSerial.printf("Sketch used percent: %.1f %%\r\n", ((float)ESP.getSketchSize() / (float)(ESP.getSketchSize() + ESP.getFreeSketchSpace())) * 100.0);
  debugSerial.printf("Used sketch space: %u Bytes\r\n", ESP.getSketchSize());
  debugSerial.printf("Total sketch space: %u Bytes\r\n", ESP.getSketchSize() + ESP.getFreeSketchSpace());
  debugSerial.printf("Flash chip size: %u Bytes\r\n", ESP.getFlashChipSize());
  debugSerial.printf("Flash chip speed: %u MHz\r\n", (uint32_t)(ESP.getFlashChipSpeed() / 1E6));
  debugSerial.print("Flash chip mode: ");
  switch (ESP.getFlashChipMode())
  {
  case FM_QIO:
    debugSerial.println("QIO");
    break;

  case FM_QOUT:
    debugSerial.println("QOUT");
    break;

  case FM_DIO:
    debugSerial.println("DIO");
    break;

  case FM_DOUT:
    debugSerial.println("DOUT");
    break;

  case FM_FAST_READ:
    debugSerial.println("fast read");
    break;

  case FM_SLOW_READ:
    debugSerial.println("slow read");
    break;

  default:
    debugSerial.println("unknown");
    break;
  }
  if (!SPIFFS.begin(true))
  {
    debugSerial.println("An error has occurred while mounting SPIFFS, system will reboot");
    ESP.restart();
    return;
  }
  xTaskCreatePinnedToCore(WIFI_task_code, "WIFI_task", 10240, NULL, 10, &WIFI_TaskHandle, 1);
  xTaskCreatePinnedToCore(LCD_task_code, "LCD_task", 5120, NULL, 10, &LCD_TaskHandle, 1);
  xTaskCreatePinnedToCore(IO_Task_code, "IO_task", 5120, NULL, 10, &IO_TaskHandle, 1);
}

void loop()
{
  uint32_t idleTime = 1E6 - WIFI_task_runTime - LCD_task_runTime - IO_task_runTime - LOOP_task_runTime;

  uint32_t current = micros();
  WIFI_task_useage_percent = (WIFI_task_runTime / 1000000.0) * 100.0;
  LCD_task_useage_percent = (LCD_task_runTime / 1000000.0) * 100.0;
  IO_task_useage_percent = (IO_task_runTime / 1000000.0) * 100.0;
  LOOP_task_useage_percent = ((LOOP_task_runTime + idleTime) / 1000000.0) * 100.0;
  Serial.printf("\r\n\r\n\r\n");
  Serial.printf("WIFI task runtime: %u uS\r\n", WIFI_task_runTime);
  Serial.printf("LCD task runtime: %u uS\r\n", LCD_task_runTime);
  Serial.printf("IO task runtime: %u uS\r\n", IO_task_runTime);
  Serial.printf("LOOP task runtime: %u uS\r\n", LOOP_task_runTime + idleTime);
  Serial.printf("WIFI task useage: %.2f%%\r\n", WIFI_task_useage_percent);
  Serial.printf("LCD task useage: %.2f%%\r\n", LCD_task_useage_percent);
  Serial.printf("IO task useage: %.2f%%\r\n", IO_task_useage_percent);
  Serial.printf("LOOP task useage: %.2f%%\r\n", LOOP_task_useage_percent);

  WIFI_task_runTime = 0;
  LCD_task_runTime = 0;
  IO_task_runTime = 0;
  LOOP_task_runTime = (int32_t)micros() - (int32_t)current;

  delay((idleTime & 0xFFFFF) / 1000);
}
