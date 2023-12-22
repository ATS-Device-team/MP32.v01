#include "shiftIO.h"

ShiftIO::ShiftIO(uint8_t numbsPortIn, uint8_t numbsPortOut)
{
    _numbsPort = (numbsPortIn >= numbsPortOut) ? numbsPortIn : numbsPortOut;

    _buffer_in = new uint8_t[_numbsPort]();
    _buffer_out = new uint8_t[_numbsPort]();
    _dummyByteOffset = numbsPortIn - numbsPortOut;
}

ShiftIO::~ShiftIO()
{
}
