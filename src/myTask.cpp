#include "myTask.h"
#include "WiFi.h"
#include "esp_task_wdt.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "wifiManager.h"
#include "webserverProcess.h"
#include "shiftIO.h"
#include "ModbusTCP.h"

#define COLUMS 20 // LCD columns
#define ROWS 4    // LCD rows
#define LCD_LIGHT_OFF_TIME 60

systemSimulator sysSimu;
LiquidCrystal_I2C LCD(PCF8574_ADDR_A20_A10_A00, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
const uint8_t wifiSignal0[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00};
const uint8_t wifiSignal1[8] = {0x00, 0x00, 0x04, 0x0A, 0x00, 0x04, 0x00, 0x00};
const uint8_t wifiSignal2[8] = {0x0E, 0x11, 0x04, 0x0A, 0x00, 0x04, 0x00, 0x00};
#if (USE_SHIFTIO_SOFT_SPI == 1)
ShiftIO muxIO(IO_CS, IO_SCLK, IO_MOSI, IO_MISO);
#else
ShiftIO muxIO(HSPI, 2E6, IO_CS, IO_SCLK, IO_MOSI, IO_MISO);
#endif
uint32_t WIFI_task_runTime = 0, PES_task_runTime = 0, LCD_task_runTime = 0, IO_task_runTime = 0;
ModbusTCP modbus;

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
    webserver_init();

    for (;;)
    {
        uint32_t current = micros();
        // if ((WiFi.status() != WL_CONNECTED) && !wifi_isConnecting())
        // {
        //     bestWifi = wifi_scan();
        //     if (bestWifi >= 0)
        //         if (wifi_connect(wifiInfor_list[bestWifi].ssid.c_str(), wifiInfor_list[bestWifi].password.c_str()))
        //             wifi_saveInfor();
        // }
        webserver_process();
        WIFI_task_runTime += (int32_t)micros() - (int32_t)current;

        delay(500);
    }
}

void PES_task_code(void *pvParameters)
{
    (void)pvParameters;
    while (WiFi.softAPIP().toString() == "0.0.0.0")
        delay(50);
    muxIO.begin(4, 4);
    modbus.server();
    for (uint8_t bitScan = 0; bitScan < 32; bitScan++)
    {
        modbus.addCoil(bitScan);
        modbus.addIsts(bitScan);
    }

    for (;;)
    {
        uint32_t current = micros();
        modbus.task();
        muxIO.transfer();
        for (uint8_t bitScan = 0; bitScan < 32; bitScan++)
        {
            modbus.Ists(bitScan, (bool)muxIO.getInputBit(bitScan));
            muxIO.setOutputBit(bitScan, modbus.Coil(bitScan));
        }
        PES_task_runTime += (int32_t)micros() - (int32_t)current;

        delay(20);
    }
}

void LCD_task_code(void *pvParameters)
{
    (void)pvParameters;

    LCD.begin(COLUMS, ROWS, LCD_5x8DOTS, I2C_SDA, I2C_SCL);
    LCD.createChar(0, wifiSignal0);
    LCD.createChar(1, wifiSignal1);
    LCD.createChar(2, wifiSignal2);
    LCD.setCursor(0, 0);
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
            LCD.print(WiFi.SSID() + " ");
            if (WiFi.RSSI() >= -60)
                LCD.write(2);
            else if (WiFi.RSSI() >= -70)
                LCD.write(1);
            else
                LCD.write(0);
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
        if (sysSimu.LCD_isTimeout())
            LCD.noBacklight();
        else
        {
            sysSimu.LCD_increTick();
            LCD.backlight();
        }
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
            sysSimu.LCD_refreshTick();
        }
        IO_task_runTime += (int32_t)micros() - (int32_t)current;

        delay(100);
    }
}

systemSimulator::systemSimulator(/* args */)
{
    _LCD_light_timeout = 60;
    _LCD_light_tick = 0;
}

systemSimulator::~systemSimulator()
{
}

bool systemSimulator::LCD_isTimeout(void)
{
    return _LCD_light_tick >= _LCD_light_timeout;
}

void systemSimulator::LCD_increTick(void)
{
    _LCD_light_tick++;
}

void systemSimulator::LCD_refreshTick(void)
{
    _LCD_light_tick = 0;
}

void systemSimulator::LCD_setTimeout(uint16_t sec)
{
    _LCD_light_timeout = (uint16_t)((float)sec / (float)LCD_TASK_INTERVAL * (float)CONFIG_FREERTOS_HZ);
}
