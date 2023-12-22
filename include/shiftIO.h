#ifndef __SHIFTIO_H__
#define __SHIFTIO_H__

#include "main.h"
#include "SPI.h"

class ShiftIO
{
private:
    uint8_t *_buffer_in;
    uint8_t *_buffer_out;
    uint8_t _numbsPort;
    int8_t _dummyByteOffset;

public:
    ShiftIO(uint8_t numbsPortIn, uint8_t numbsPortOut);
    ~ShiftIO();
};

#endif
