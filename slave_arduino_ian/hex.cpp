#include "hex.h"

long x2i(char *s)
{
    long x = 0;
    for (;;)
    {
        char c = *s;
        if (c >= '0' && c <= '9')
        {
            x *= 16;
            x += c - '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            x *= 16;
            x += (c - 'A') + 10;
        }
        else if (c >= 'a' && c <= 'f')
        {
            x *= 16;
            x += (c - 'a') + 10;
        }
        else
            break;
        s++;
    }
    return x;
}

long x2i(const char *s)
{
    long x = 0;
    for (;;)
    {
        char c = *s;
        if (c >= '0' && c <= '9')
        {
            x *= 16;
            x += c - '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            x *= 16;
            x += (c - 'A') + 10;
        }
        else if (c >= 'a' && c <= 'f')
        {
            x *= 16;
            x += (c - 'a') + 10;
        }
        else
            break;
        s++;
    }
    return x;
}

unsigned long x2u(char *s)
{
    unsigned long x = 0;
    for (;;)
    {
        char c = *s;
        if (c >= '0' && c <= '9')
        {
            x *= 16;
            x += c - '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            x *= 16;
            x += (c - 'A') + 10;
        }
        else if (c >= 'a' && c <= 'f')
        {
            x *= 16;
            x += (c - 'a') + 10;
        }
        else
            break;
        s++;
    }
    return x;
}

unsigned long x2u(const char *s)
{
    unsigned long x = 0;
    for (;;)
    {
        char c = *s;
        if (c >= '0' && c <= '9')
        {
            x *= 16;
            x += c - '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            x *= 16;
            x += (c - 'A') + 10;
        }
        else if (c >= 'a' && c <= 'f')
        {
            x *= 16;
            x += (c - 'a') + 10;
        }
        else
            break;
        s++;
    }
    return x;
}