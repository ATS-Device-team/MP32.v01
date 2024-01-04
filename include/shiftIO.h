#ifndef __SHIFTIO_H__
#define __SHIFTIO_H__

#include "main.h"
#include "SPI.h"

#define USE_SHIFTIO_SOFT_SPI 0

class ShiftIO
{
private:
    uint8_t *_buffer_in;
    uint8_t *_buffer_out;
    uint8_t _numbsPort;
    int8_t _dummyByteOffset;
#if (USE_SHIFTIO_SOFT_SPI == 1)
    uint8_t _ctrlPin;
    uint8_t _clkPin;
    uint8_t _output595Pin;
    uint8_t _input165Pin;
#else
    SPIClass *_spi = NULL;
    uint32_t _speed;
#endif

public:
#if (USE_SHIFTIO_SOFT_SPI == 1)
    ShiftIO(uint8_t ctrlPin, uint8_t clkPin, uint8_t output595Pin, uint8_t input165Pin);
#else
    ShiftIO(uint8_t spi, uint32_t speed, uint8_t ctrlPin, uint8_t clkPin, uint8_t output595Pin, uint8_t input165Pin);
    void setSpeed(uint32_t speed);
#endif
    ~ShiftIO();
    void begin(uint8_t numbsPortIn, uint8_t numbsPortOut);
    void resizePort(uint8_t numbsPortIn, uint8_t numbsPortOut);
    uint8_t getInputPortNumbs(void);
    uint8_t getOutputPortNumbs(void);
    void transfer();
    int setOutputPort(uint8_t portAddress, uint8_t data);
    int getOutputPort(uint8_t portAddress);
    int getInputPort(uint8_t portAddress);
    int setOutputBit(uint16_t bitAddress, bool bitVal);
    int getOutputBit(uint16_t bitAddress);
    int getInputBit(uint16_t bitAddress);
};

#endif
