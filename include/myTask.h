#include "main.h"

class systemSimulator
{
private:
public:
    systemSimulator(/* args */);
    ~systemSimulator();
};

extern uint32_t WIFI_task_runTime, LCD_task_runTime, IO_task_runTime;

void WIFI_task_code(void *pvParameters);
void LCD_task_code(void *pvParameters);
void IO_Task_code(void *pvParameters);
