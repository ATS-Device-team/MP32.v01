#include "shiftIO.h"

#if (USE_SHIFTIO_SOFT_SPI == 1)
ShiftIO::ShiftIO(uint8_t ctrlPin, uint8_t clkPin, uint8_t output595Pin, uint8_t input165Pin)
{
    _ctrlPin = ctrlPin;
    _clkPin = clkPin;
    _output595Pin = output595Pin;
    _input165Pin = input165Pin;
    pinMode(_ctrlPin, OUTPUT);
    pinMode(_clkPin, OUTPUT);
    pinMode(_output595Pin, OUTPUT);
    pinMode(_input165Pin, INPUT);
    digitalWrite(_ctrlPin, LOW);
    digitalWrite(_clkPin, LOW);
    digitalWrite(_output595Pin, LOW);
}
#else
ShiftIO::ShiftIO(uint8_t spi, uint32_t speed, uint8_t ctrlPin, uint8_t clkPin, uint8_t output595Pin, uint8_t input165Pin)
{
    _spi = new SPIClass(spi);
    _spi->begin(clkPin, input165Pin, output595Pin, ctrlPin);
    _spi->setHwCs(false);
    pinMode(_spi->pinSS(), OUTPUT);
    digitalWrite(_spi->pinSS(), HIGH);
    _speed = speed;
}

void ShiftIO::setSpeed(uint32_t speed)
{
    _speed = speed;
}
#endif

ShiftIO::~ShiftIO()
{
    delete[] _buffer_in;
    delete[] _buffer_out;
}

void ShiftIO::begin(uint8_t numbsPortIn, uint8_t numbsPortOut)
{
    _dummyByteOffset = numbsPortIn - numbsPortOut;
    _numbsPort = (_dummyByteOffset >= 0) ? numbsPortIn : numbsPortOut;
    _buffer_in = new uint8_t[_numbsPort]();
    _buffer_out = new uint8_t[_numbsPort]();
}

void ShiftIO::resizePort(uint8_t numbsPortIn, uint8_t numbsPortOut)
{
    delete[] _buffer_in;
    delete[] _buffer_out;
    _dummyByteOffset = numbsPortIn - numbsPortOut;
    _numbsPort = (_dummyByteOffset >= 0) ? numbsPortIn : numbsPortOut;
    _buffer_in = new uint8_t[_numbsPort]();
    _buffer_out = new uint8_t[_numbsPort]();
}

uint8_t ShiftIO::getInputPortNumbs(void)
{
    return (_dummyByteOffset >= 0) ? _numbsPort : _numbsPort + _dummyByteOffset;
}

uint8_t ShiftIO::getOutputPortNumbs(void)
{
    return (_dummyByteOffset <= 0) ? _numbsPort : _numbsPort - _dummyByteOffset;
}

void ShiftIO::transfer()
{
#if (USE_SHIFTIO_SOFT_SPI == 1)
    digitalWrite(_ctrlPin, HIGH);
    for (uint8_t byteScan = 0; byteScan < _numbsPort; byteScan++)
    {
        _buffer_in[byteScan] = 0;
        for (uint8_t bitScan = 0; bitScan < 8; bitScan++)
        {
            digitalWrite(_clkPin, LOW);
            if (digitalRead(_input165Pin))
                _buffer_in[byteScan] |= 0x80 >> bitScan;
            digitalWrite(_output595Pin, _buffer_out[byteScan] & (0x80 >> bitScan));
            digitalWrite(_clkPin, HIGH);
        }
    }
    digitalWrite(_clkPin, LOW);
    digitalWrite(_output595Pin, LOW);
    digitalWrite(_ctrlPin, LOW);
    digitalWrite(_ctrlPin, HIGH);
    digitalWrite(_ctrlPin, LOW);
#else
    digitalWrite(_spi->pinSS(), HIGH);
    _spi->beginTransaction(SPISettings(_speed, MSBFIRST, SPI_MODE0));
    _spi->transferBytes(_buffer_out, _buffer_in, _numbsPort);
    _spi->endTransaction();
    digitalWrite(_spi->pinSS(), LOW);
    digitalWrite(_spi->pinSS(), HIGH);
    digitalWrite(_spi->pinSS(), LOW);
#endif
}

int ShiftIO::setOutputPort(uint8_t portAddress, uint8_t data)
{
    if (portAddress >= getOutputPortNumbs())
        return -1;
    portAddress = _numbsPort - 1 - portAddress;
    _buffer_out[portAddress] = data;
    return _buffer_out[portAddress];
}

int ShiftIO::getOutputPort(uint8_t portAddress)
{
    if (portAddress >= getOutputPortNumbs())
        return -1;
    portAddress = _numbsPort - 1 - portAddress;
    return _buffer_out[portAddress];
}

int ShiftIO::getInputPort(uint8_t portAddress)
{
    if (portAddress >= getInputPortNumbs())
        return -1;
    return _buffer_in[portAddress];
}

int ShiftIO::setOutputBit(uint16_t bitAddress, bool bitVal)
{
    uint8_t byteAddress = bitAddress / 8;
    if (byteAddress >= getOutputPortNumbs())
        return -1;
    bitAddress %= 8;
    byteAddress = _numbsPort - 1 - byteAddress;
    if (bitVal)
        _buffer_out[byteAddress] |= 0x01 << bitAddress;
    else
        _buffer_out[byteAddress] &= ~(0x01 << bitAddress);
    return bitVal;
}

int ShiftIO::getOutputBit(uint16_t bitAddress)
{
    uint8_t byteAddress = bitAddress / 8;
    if (byteAddress >= getOutputPortNumbs())
        return -1;
    bitAddress %= 8;
    byteAddress = _numbsPort - 1 - byteAddress;
    if ((_buffer_out[byteAddress] & (0x01 << bitAddress)) == (0x01 << bitAddress))
        return 1;
    else
        return 0;
}

int ShiftIO::getInputBit(uint16_t bitAddress)
{
    uint8_t byteAddress = bitAddress / 8;
    if (byteAddress >= getInputPortNumbs())
        return -1;
    bitAddress %= 8;
    if ((_buffer_in[byteAddress] & (0x01 << bitAddress)) == (0x01 << bitAddress))
        return 1;
    else
        return -1;
}
