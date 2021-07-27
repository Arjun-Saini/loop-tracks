/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/Arjun/Documents/GitHub/loop_tracks/master/src/master.ino"
void setup();
void loop();
void randomizeAddress();
#line 1 "c:/Users/Arjun/Documents/GitHub/loop_tracks/master/src/master.ino"
SYSTEM_MODE(MANUAL)

int slaveCountExpected = 2;
int slaveCount;

void setup() {
  Serial.begin(9600);
  delay(2000);
  Wire.begin();
  randomizeAddress();
}

void loop() {
  delay(500);
}

void randomizeAddress(){
  while(slaveCount != slaveCountExpected){
    //scan through each i2c address
    slaveCount = 0;
    for(int i = 8; i <= 119; i++){
      Serial.println("\nrequest code 1, address: " + String(i));
      Wire.beginTransmission(i);
      Wire.write('1');
      Wire.endTransmission();

      Wire.requestFrom(i, 24);
      if(Wire.available() > 0){
        Serial.println("transmission recieved from: " + String(i));

        slaveCount++;

        String inputBuffer = "";
        char c;
        for(int j = 0; j < 24; j++){
          c = Wire.read();
          inputBuffer += c;
        }
        Wire.beginTransmission(i);
        Wire.write(inputBuffer);
        Serial.println("device id: " + inputBuffer);
        Wire.endTransmission();
        Serial.println("transmission sent to: " + String(i));

        Wire.beginTransmission(i);
        Wire.write('2');
        Wire.endTransmission();

        Serial.println("request code 2, address: " + String(i));
        Wire.requestFrom(i, 4);
        inputBuffer = "";
        for(int j = 0; j < 4; j++){
          inputBuffer += (char)Wire.read();
        }
        
        Serial.println("conflict verification: " + inputBuffer);
      }
    }
  }

  Serial.println("Connected to: ");
  for(int i = 8; i<= 119; i++){
    Wire.beginTransmission(i);
    Wire.write('1');
    Wire.endTransmission();

    Wire.requestFrom(i, 24);
    if(Wire.available() > 0){
      Serial.print(i);
      Serial.print(", ");
    }
  }
}