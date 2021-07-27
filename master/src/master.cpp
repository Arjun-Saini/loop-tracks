/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "e:/IoT/loop-tracks/master/src/master.ino"
void setup();
void loop();
#line 1 "e:/IoT/loop-tracks/master/src/master.ino"
SYSTEM_MODE(MANUAL)

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  byte transmission = 1;
  Wire.beginTransmission(0x20);
  Wire.write("transmission");
  Wire.endTransmission();

  Wire.requestFrom(0x20, 7);
  Serial.println(Wire.available());
  delay(500);
}