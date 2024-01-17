#include "main.h"

#define WIFI_TASK_INTERVAL
#define PES_TASK_INTERVAL
#define LCD_TASK_INTERVAL 500
#define IO_TASK_INTERVAL

class systemSimulator
{
private:
    uint16_t _LCD_light_tick;
    uint16_t _LCD_light_timeout;

public:
    systemSimulator(/* args */);
    ~systemSimulator();
    bool LCD_isTimeout(void);
    void LCD_increTick(void);
    void LCD_refreshTick(void);
    void LCD_setTimeout(uint16_t sec);
};

extern systemSimulator sysSimu;
extern uint32_t WIFI_task_runTime, PES_task_runTime, LCD_task_runTime, IO_task_runTime;

void WIFI_task_code(void *pvParameters);
void PES_task_code(void *pvParameters);
void LCD_task_code(void *pvParameters);
void IO_Task_code(void *pvParameters);
