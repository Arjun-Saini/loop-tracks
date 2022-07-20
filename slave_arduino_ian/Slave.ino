#include <Arduino.h>
#include "hex.h"

void setup()
{
    // Initialize the serial port:
    Serial.begin(9600);
}

void loop()
{
    Serial.println("Initializing...");
    Serial.println(x2u("abcdef"), HEX);
    Serial.println("Done.");
    delay(1000);
}