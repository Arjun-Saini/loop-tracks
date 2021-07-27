/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "e:/IoT/loop-tracks/slave/src/slave.ino"
void setup();
void loop();
void dataReceived(int count);
void dataRequest();
#line 1 "e:/IoT/loop-tracks/slave/src/slave.ino"
SYSTEM_MODE(MANUAL)

void setup() {
  Serial.begin(9600);
  Wire.begin(0x20);
  Wire.onReceive(dataReceived);
  Wire.onRequest(dataRequest);
}

void loop() {
  Serial.println(Wire.available());
  delay(100);
}

void dataReceived(int count){
  Serial.println("data received");
}

void dataRequest(){
  Serial.println("request received");
  Wire.write("request");
}