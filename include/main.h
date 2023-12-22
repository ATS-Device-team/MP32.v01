#ifndef __MAIN_H__
#define __MAIN_H__

#include "Arduino.h"

#define MEMBER_COUNT(A) (sizeof(A) / sizeof(A[0]))
#define timeReachedMacro(current, last, duration) (((int32_t)current - (int32_t)last) >= (int32_t)duration)

#define debugSerial Serial
#define I2C_SDA 33
#define I2C_SCL 32
#define BUTTON1 35
#define BUTTON2 34
#define BUTTON3 36
#define BUTTON4 39

#endif