/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/Arjun/Documents/GitHub/loop_tracks/slave/src/slave.ino"
void setup();
void loop();
void dataReceived(int count);
void dataRequest();
#line 1 "c:/Users/Arjun/Documents/GitHub/loop_tracks/slave/src/slave.ino"
SYSTEM_MODE(MANUAL)

int address;
int requestMode = 0;
bool verifyAddress = false;
String deviceID = System.deviceID();

void setup() {
  Serial.begin(9600);
  address = random(8, 64);
  Wire.begin(15);
  Wire.onReceive(dataReceived);
  Wire.onRequest(dataRequest);
}

void loop() {
  delay(100);
}

char c;

void dataReceived(int count){
  char inputBuffer[Wire.available()];
  int counter = 0;

  Serial.println("count: ");
  Serial.print(Wire.available());
  Serial.println();

  while(Wire.available() > 0){
    c = Wire.read();
    Serial.println(c);
    inputBuffer[counter] = c;
    counter++;
  }
  Serial.println("data received: ");
  Serial.print(inputBuffer);
  Serial.println();

  if(inputBuffer[0] == '1'){
    requestMode = 1;
  }else if(inputBuffer[0] == '2'){
    requestMode = 2;
  }else{
    for(int i = 0; i < 24; i++){
      if(deviceID.charAt(i) != inputBuffer[i]){
        verifyAddress = false;
        break;
      }
      verifyAddress = true;
    }
    if(verifyAddress){
      Serial.println("correct address");
    }else{
      Serial.println("randomize address");
      address = random(64, 120);
      Wire.end();
      Wire.begin(address);
      Wire.onReceive(dataReceived);
      Wire.onRequest(dataRequest);
      requestMode = 0;
    }
  }
}

void dataRequest(){
  Serial.println("request received");
  switch(requestMode){
    case 1:{
      Serial.println("request mode 1");
      Wire.write(deviceID);
      break;
    }
    case 2:{
      Serial.println("request mode 2");
      if(verifyAddress){
        Wire.write("pass");
        verifyAddress = false;
      }else{
        Wire.write("fail");
      }
      break;
    }
  }
}