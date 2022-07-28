// #include <ArduinoUniqueID.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "src/Wire/src/Wire.h" // local copy of Wire.h so we can override the buffer size limit

#define MAX_PIXELS 100
#define CHALLENGE_LEN 25 // 24 + 1 for null terminator
#define BLINK LED_BUILTIN
#define UPDATE A1

volatile int address;
volatile bool verifyAddress = true;
volatile int requestMode = 0;
volatile bool blink = false;
volatile bool clear = false;
volatile bool refresh = false;
volatile uint32_t headColor;
volatile uint32_t tailColor;

char challenge[CHALLENGE_LEN];

Adafruit_NeoPixel strip(MAX_PIXELS, 2, NEO_GRB + NEO_KHZ800);

void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);

void setup()
{

    Serial.begin(115200);

    // initialize randomness w/ analog read of pin A0
    // instead of Unique ID, as it doesn't work for some reason
    randomSeed(analogRead(0));

    // adds the rest of the random digits to the device id
    for (size_t i = 0; i < CHALLENGE_LEN; i++)
    {
        challenge[i] = (char)random('0', '9');
    }

    challenge[CHALLENGE_LEN - 1] = '\0';

    // seeds with the device id
    address = random(0x00, 0x69);

    // address 41 is for VL6180
    while (address == 0x29)
    {
        address = random(0x00, 0x69);
    }

    Wire.setClock(400000);
    Wire.begin(address);
    Wire.onReceive(dataReceived);
    Wire.onRequest(dataRequest);

    strip.begin();
    strip.clear();
    strip.show();
}

void loop()
{
    if (clear)
    {
        strip.clear();
        strip.show();
        clear = false;
    }

    if (blink)
    {
        rainbow(5);
    }

    if (!verifyAddress)
    {
        Serial.print(F("randomize address: "));
        Serial.println(address);
        address = random(0x00, 0x69);

        while (address == 0x29)
        {
            address = random(0x00, 0x69);
        }

        Serial.println(address);

        Wire.end();
        Wire.setClock(400000);
        Wire.begin(address);
        Wire.onReceive(dataReceived);
        Wire.onRequest(dataRequest);
        requestMode = 0;
        verifyAddress = true;
    }

    if (refresh)
    {
        refresh = false;
        strip.show();
    }
}

void dataReceived(int count)
{
    int size = Wire.available();
    char inputBuffer[size];
    int counter = 0;
    Serial.print("Data recieved: ");
    while (Wire.available() > 0)
    {
        char c = Wire.read();
        inputBuffer[counter] = c;
        counter++;
    }
    inputBuffer[counter] = '\0';
    Serial.println(inputBuffer);

    if (size == 1)
    {
        if (inputBuffer[0] == '1')
        {
            requestMode = 1;
            return;
        }
        else if (inputBuffer[0] == '2')
        {
            requestMode = 2;
            return;
        }
        else if (inputBuffer[0] == '3')
        {
            blink = true;
            return;
        }
        else if (inputBuffer[0] == '4')
        {
            blink = false;
            clear = true;
            return;
        }
        else if (inputBuffer[0] == '5')
        {
            // do OTA updates here
            return;
        }
    }
    else if (size == 24)
    {
        for (int i = 0; i < 24; i++)
        {
            if (challenge[i] != inputBuffer[i])
            {
                verifyAddress = false;
                return;
            }
        }
    }
    else if (size == 12)
    {
        char headBuffer[7];
        char tailBuffer[7];
        for (int i = 0; i < 6; i++)
        {
            headBuffer[i] = inputBuffer[i];
            tailBuffer[i] = inputBuffer[i + 6];
        }
        headBuffer[6] = '\0';
        tailBuffer[6] = '\0';
        headColor = strtoul(headBuffer, NULL, 16);
        tailColor = strtoul(tailBuffer, NULL, 16);
        return;
    }
    else
    {
        for (int i = 0; i < size + 1; i++)
        {
            if (strip.getPixelColor(i) == headColor || strip.getPixelColor(i) == tailColor)
            {
                strip.setPixelColor(i, 0);
            }
        }
        for (int i = 0; i < size; i++)
        {
            if (inputBuffer[i] == '1')
            {
                strip.setPixelColor(i - 1, tailColor);
                strip.setPixelColor(i, tailColor);
                strip.setPixelColor(i + 1, headColor);
            }
            else if (inputBuffer[i] == '5')
            {
                strip.setPixelColor(i - 1, headColor);
                strip.setPixelColor(i, tailColor);
                strip.setPixelColor(i + 1, tailColor);
            }
        }
        refresh = true;
        return;
    }
}

void dataRequest()
{
    // Serial.println("request received");
    switch (requestMode)
    {
    case 1:
    {
        // Serial.println("request mode 1");
        Wire.write(challenge);
        break;
    }
    case 2:
    {
        // Serial.println("request mode 2");
        // Serial.println((unsigned long)UniqueID);
        if (verifyAddress)
        {
            Wire.write("pass");
        }
        else
        {
            Wire.write("fail");
        }
        break;
    }
    }
}

void rainbow(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256; j++)
    {
        for (i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, Wheel((i + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
    if (WheelPos < 85)
    {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
    else if (WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else
    {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}