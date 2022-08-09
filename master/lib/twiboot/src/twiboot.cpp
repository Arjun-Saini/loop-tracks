#include "Particle.h"
#include "twiboot.h"
#include "crc.h"

inline void startWire()
{
    if (!Wire.isEnabled())
    {
        Wire.begin();
    }
}

int numPagesInLen(int len)
{
    if (len % PAGE_SIZE == 0)
    {
        return len / PAGE_SIZE;
    }
    else
    {
        return (len / PAGE_SIZE) + 1;
    }
}

Twiboot::Twiboot()
{
    this->addr = 0x29;
    startWire();
}

Twiboot::Twiboot(uint8_t address)
{
    this->addr = address;
    startWire();
}

bool Twiboot::AbortBootTimeout()
{
    startWire();

    Wire.beginTransmission(addr);
    Wire.write(0x00);
    return Wire.endTransmission() == 0; // if there are any errors, return false. Otherwise, return true.
}

void Twiboot::GetBootloaderVersion(char *buf)
{
    startWire();
    Wire.beginTransmission(addr);
    Wire.write(0x01);
    Wire.endTransmission();
    Wire.requestFrom(addr, 16);
    int i = 0;
    while (Wire.available())
    {
        buf[i] = Wire.read();
        i++;
    }
}

void Twiboot::GetChipInfo(uint64_t *signature, uint8_t *pageSize, uint16_t *flashSize, uint16_t *eepromSize)
{
    startWire();
    Wire.beginTransmission(addr);
    Wire.write(0x02);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
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
}

void Twiboot::ReadFlashPage(uint16_t page, uint8_t *buf)
{
    startWire();
    Wire.beginTransmission(addr);
    Wire.write(0x02);
    Wire.write(0x01);
    Wire.write((page * PAGE_SIZE) >> 8 & 0xFF);
    Wire.write((page * PAGE_SIZE) & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom(addr, PAGE_SIZE);

    for (int i = 0; i < PAGE_SIZE; i++)
    {
        buf[i] = Wire.read();
    }
}

// uint8_t Twiboot::ReadEEPROMByte(uint16_t addr)
// {
//     startWire();
//     Wire.beginTransmission(addr);
//     Wire.write(0x02);
//     Wire.write(0x02);
//     Wire.write((addr >> 8) & 0xFF);
//     Wire.write(addr & 0xFF);
//     Wire.endTransmission();
//     Wire.requestFrom(addr, 1);

//     return Wire.read();
// }

// uint8_t Twiboot::WriteEEPROMBytes(uint16_t addr, uint8_t *buf, uint8_t len)
// {
//     startWire();
//     Wire.beginTransmission(addr);
//     Wire.write(0x02);
//     Wire.write(0x02);
//     Wire.write(addr >> 8 & 0xFF);
//     Wire.write(addr & 0xFF);
//     Wire.write(buf, len);
//     return Wire.endTransmission();

//     delay(21); // wait for the flash to finish
// }

void Twiboot::WriteFlash(uint8_t *buf, int len, uint16_t page)
{
    startWire();

    int numPages = numPagesInLen(len);

    for (int i = 0; i < numPages; i++)
    {
        Wire.beginTransmission(addr);
        Wire.write(0x02);
        Wire.write(0x01);
        Wire.write(((i + page) * PAGE_SIZE) >> 8 & 0xFF);
        Wire.write(((i + page) * PAGE_SIZE) & 0xFF);

        for (int j = i * PAGE_SIZE; j < (i + 1) * PAGE_SIZE; j++)
        {
            if ((i == numPages - 1) && ((len % PAGE_SIZE) != 0) && j >= len)
            {
                Wire.write(0xFF);
            }
            else
            {
                Wire.write(buf[j]);
            }
        }

        Wire.endTransmission();

        delay(21); // wait for the flash to finish
    }
}

bool Twiboot::Verify(uint8_t *buf, int len, uint16_t page)
{
    crcInit(); // has to be called before crcFast() to update the CRC tables

    for (int i = 0; i < numPagesInLen(len); i++)
    {
        uint8_t read[PAGE_SIZE];
        uint8_t tbuf[PAGE_SIZE];
        startWire();
        Wire.beginTransmission(addr);
        Wire.write(0x02);
        Wire.write(0x01);
        Wire.write(((i + page) * PAGE_SIZE) >> 8 & 0xFF);
        Wire.write(((i + page) * PAGE_SIZE) & 0xFF);
        Wire.endTransmission();
        Wire.requestFrom(addr, PAGE_SIZE);

        for (int j = 0; j < PAGE_SIZE; j++)
        {

            while (!Wire.available())
                ;

            if ((i * PAGE_SIZE + j) >= len)
            {
                read[j] = 0xFF;
                tbuf[j] = 0xFF;
            }
            else
            {
                read[j] = Wire.read();
                tbuf[j] = buf[i * PAGE_SIZE + j];
            }
        }

        if (crcFast(read, PAGE_SIZE) != crcFast(tbuf, PAGE_SIZE))
        {
            return false;
        }
    }

    return true;
}

void Twiboot::JumpToApp()
{
    startWire();

    Wire.beginTransmission(addr);
    Wire.write(0x01);
    Wire.write(0x80);
    Wire.endTransmission();
}
