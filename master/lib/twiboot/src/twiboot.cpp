#include "Particle.h"
#include "twiboot.h"
#include "crc.h"

Twiboot::Twiboot()
{
    START_WIRE;
    this->addr = 0x29;
}

Twiboot::Twiboot(uint8_t address)
{
    START_WIRE;
    this->addr = address;
}

bool Twiboot::Init()
{
    START_WIRE;
    Wire.beginTransmission(addr);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) // if there are any errors, return false. Otherwise, return true.
        return false;

    uint8_t *pgsz = &this->page_size;

    return GetChipInfo(nullptr, pgsz, nullptr, nullptr);
}

bool Twiboot::GetBootloaderVersion(char *buf)
{
    Wire.beginTransmission(addr);
    Wire.write(0x01);

    if (Wire.endTransmission() != 0) // if there are any errors, return false. Otherwise, return true.
        return false;

    Wire.requestFrom(addr, 16);
    int i = 0;
    while (Wire.available())
    {
        buf[i] = Wire.read();
        i++;
    }

    return i == 16;
}

bool Twiboot::GetChipInfo(uint64_t *signature, uint8_t *pageSize, uint16_t *flashSize, uint16_t *eepromSize)
{
    Wire.beginTransmission(addr);
    Wire.write(0x02);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0x00);

    if (Wire.endTransmission() != 0) // if there are any errors, return false. Otherwise, return true.
        return false;

    Wire.requestFrom(addr, 8);
    while (!Wire.available())
        ;
    *signature = Wire.read() << 16;
    *signature |= Wire.read() << 8;
    *signature |= Wire.read();
    *pageSize = Wire.read();
    *flashSize = Wire.read() << 8;
    *flashSize |= Wire.read();
    *eepromSize = Wire.read() << 8;
    *eepromSize |= Wire.read();

    return true;
}

// uint8_t Twiboot::ReadEEPROMByte(uint16_t addr)
// {
//     byte tmp[4] = {
//         0x02,
//         0x02,
//         (uint8_t)((addr >> 8) & 0xFF),
//         (uint8_t)((addr)&0xFF),
//     };

//     Wire.beginTransmission(this->addr);
//     Wire.write(tmp, 4);
//     if (Wire.endTransmission() != 0) // if there are any errors, return false. Otherwise, return true.
//         return false;

//     Wire.requestFrom(this->addr, 1);

//     while (!Wire.available())
//         ;

//     return Wire.read();
// }

// bool Twiboot::WriteEEPROM(uint8_t *buf, uint8_t len, uint16_t addr)
// {

//     int numPages = NUM_PAGES_IN(len);

//     for (int i = 0; i < numPages; i++)
//     {
//         byte tmp[page_size + 4] = {
//             0x02,
//             0x02,
//             (uint8_t)((addr >> 8) & 0xFF),
//             (uint8_t)((addr)&0xFF),
//         };

//         for (int j = 0; (i * page_size + j) < len; j++)
//         {
//             tmp[j + 4] = buf[i * page_size + j];
//         }
//         Wire.beginTransmission(this->addr);
//         Wire.write(tmp, page_size + 4);
//         if (Wire.endTransmission() != 0)
//             return false;

//         delay(20); // wait for the flash to finish
//     }

//     return true;
// }

bool Twiboot::ReadFlashPage(uint8_t *buf, uint16_t page)
{

    byte tmp[4] = {
        0x02,
        0x01,
        (uint8_t)((page * page_size) >> 8 & 0xFF),
        (uint8_t)((page * page_size) & 0xFF),
    };

    Wire.beginTransmission(addr);
    Wire.write(tmp, 4);

    if (Wire.endTransmission() != 0) // if there are any errors, return false. Otherwise, return true.
        return false;

    Wire.requestFrom(this->addr, page_size);

    for (int i = 0; i < page_size; i++)
    {
        buf[i] = Wire.read();
    }

    return true;
}

bool Twiboot::WriteFlash(uint8_t *buf, int len, uint16_t page)
{
    int numPages = NUM_PAGES_IN(len);

    for (int i = 0; i < numPages; i++)
    {
        byte tmp[page_size + 4] = {
            0x02,
            0x01,
            (uint8_t)((((i + page) * page_size) >> 8) & 0xFF),
            (uint8_t)(((i + page) * page_size) & 0xFF),
        };

        for (int j = 0; j < page_size; j++)
        {
            if (i * page_size + j < len)
            {
                tmp[j + 4] = buf[i * page_size + j];
            }
            else
            {
                tmp[j + 4] = 0xFF;
            }
        }

        Wire.beginTransmission(addr);
        Wire.write(tmp, page_size + 4);
        if (Wire.endTransmission() != 0)
            return false;

        delay(20); // wait for the flash to finish
    }

    return true;
}

bool Twiboot::Verify(uint8_t *buf, int len, uint16_t page)
{
    for (int i = 0; i < NUM_PAGES_IN(len); i++)
    {
        uint8_t read[page_size];
        uint8_t tbuf[page_size];

        if (!ReadFlashPage(read, i + page))
            return false;

        for (int j = 0; j < page_size; j++)
        {
            if ((i * page_size + j) >= len || read[j] == 0xFF)
            {
                tbuf[j] = 0xFF;
            }
            else
            {
                tbuf[j] = buf[i * page_size + j];
            }
        }

        if (crcFast(read, page_size) != crcFast(tbuf, page_size))
        {
            return false;
        }
    }

    return true;
}

bool Twiboot::Exit()
{
    Wire.beginTransmission(addr);
    Wire.write(0x01);
    Wire.write(0x80);
    if (Wire.endTransmission() != 0)
        return false;
    Wire.end();
    return true;
}
