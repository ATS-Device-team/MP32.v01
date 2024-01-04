#include "myTask.h"
#include "WiFi.h"
#include "esp_task_wdt.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "wifiManager.h"
#include "shiftIO.h"

#define COLUMS 20 // LCD columns
#define ROWS 4    // LCD rows
#define LCD_LIGHT_OFF_TIME 60

LiquidCrystal_I2C LCD(PCF8574_ADDR_A20_A10_A00, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
#if (USE_SHIFTIO_SOFT_SPI == 1)
ShiftIO muxIO(IO_CS, IO_SCLK, IO_MOSI, IO_MISO);
#else
ShiftIO muxIO(HSPI, 2E6, IO_CS, IO_SCLK, IO_MOSI, IO_MISO);
#endif
uint8_t LCD_light_tick = 0;
uint32_t WIFI_task_runTime = 0, LCD_task_runTime = 0, IO_task_runTime = 0;

void WIFI_task_code(void *pvParameters)
{
    (void)pvParameters;
    int bestWifi;
    uint32_t scanWifi_Tick = 0;

    esp_task_wdt_init(30, true);
    wifi_loadInfor();
    wifi_init();
    bestWifi = wifi_scan();
    if (bestWifi >= 0)
        if (wifi_connect(wifiInfor_list[bestWifi].ssid.c_str(), wifiInfor_list[bestWifi].password.c_str()))
            wifi_saveInfor();
    for (;;)
    {
        uint32_t current = micros();
        if (WiFi.status() != WL_CONNECTED)
        {
            bestWifi = wifi_scan();
            if (bestWifi >= 0)
                if (wifi_connect(wifiInfor_list[bestWifi].ssid.c_str(), wifiInfor_list[bestWifi].password.c_str()))
                    wifi_saveInfor();
        }
        WIFI_task_runTime += (int32_t)micros() - (int32_t)current;

        delay(500);
    }
}

void LCD_task_code(void *pvParameters)
{
    (void)pvParameters;

    LCD.begin(COLUMS, ROWS, LCD_5x8DOTS, I2C_SDA, I2C_SCL);
    LCD.print(F("Run time: "));
    LCD.setCursor(0, 1);
    LCD.print(F("WIFI "));
    LCD.setCursor(0, 2);
    LCD.print(F("SSID: "));
    LCD.setCursor(0, 3);
    LCD.print(F("IP: "));

    for (;;)
    {
        uint32_t current = micros();
        LCD.setCursor(9, 0);
        LCD.printf("%us", millis() / 1000);
        LCD.setCursor(5, 1);
        if (WiFi.status() == WL_CONNECTED)
        {
            LCD.print(F("connected"));
            LCD.setCursor(6, 2);
            LCD.print(WiFi.SSID());
            LCD.printf(" %d", WiFi.RSSI());
            LCD.setCursor(4, 3);
            LCD.print(WiFi.localIP().toString());
        }
        else
        {
            LCD.print(F("not connected"));
            LCD.setCursor(6, 2);
            LCD.print(F("              "));
            LCD.setCursor(4, 3);
            LCD.print(F("                "));
        }
        if (LCD_light_tick < LCD_LIGHT_OFF_TIME)
        {
            LCD_light_tick++;
            LCD.backlight();
        }
        else
            LCD.noBacklight();
        LCD_task_runTime += (int32_t)micros() - (int32_t)current;

        delay(500);
    }
}

void IO_Task_code(void *pvParameters)
{
    (void)pvParameters;

    pinMode(BUTTON1, INPUT);
    pinMode(BUTTON2, INPUT);
    pinMode(BUTTON3, INPUT);
    pinMode(BUTTON4, INPUT);
    muxIO.begin(4, 4);
    muxIO.setOutputPort(0, 0x01);

    for (;;)
    {
        uint32_t current = micros();
        if (digitalRead(BUTTON1) == 0)
        {
            delay(200);
            while (digitalRead(BUTTON1) == 0)
                delay(10);
            wifi_default();
            wifi_saveInfor();
        }
        if (digitalRead(BUTTON2) == 0)
        {
            delay(200);
            while (digitalRead(BUTTON2) == 0)
                delay(10);
            if (wifi_connect("ATS106", "66668888"))
                wifi_saveInfor();
        }
        if (digitalRead(BUTTON4) == 0)
        {
            delay(200);
            while (digitalRead(BUTTON4) == 0)
                delay(10);
            if (wifi_connect("ATS-206", "Atsjsc123"))
                wifi_saveInfor();
        }
        if (digitalRead(BUTTON3) == 0)
        {
            delay(200);
            while (digitalRead(BUTTON3) == 0)
                delay(10);
            LCD_light_tick = 0;
        }
        muxIO.setOutputPort(0, (uint8_t)muxIO.getInputPort(0));
        muxIO.setOutputPort(1, (uint8_t)muxIO.getInputPort(1));
        muxIO.setOutputPort(2, (uint8_t)muxIO.getInputPort(2));
        muxIO.setOutputPort(3, (uint8_t)muxIO.getInputPort(3));
        muxIO.transfer();
        IO_task_runTime += (int32_t)micros() - (int32_t)current;

        delay(50);
    }
}

systemSimulator::systemSimulator(/* args */)
{
}

systemSimulator::~systemSimulator()
{
}
