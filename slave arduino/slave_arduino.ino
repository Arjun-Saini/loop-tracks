#include <ArduinoUniqueID.h>
#include <Wire.h>
#include <ArduinoUniqueID.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

//SYSTEM_MODE(MANUAL);

#define MAX_PIXELS 100

int address;
int requestMode = 0;
bool verifyAddress = true;
String deviceID = "";

bool blink = false;
Adafruit_NeoPixel strip(MAX_PIXELS, 2, NEO_GRB + NEO_KHZ800);

uint32_t headColor;
uint32_t tailColor;

void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);

void setup() {
  Serial.begin(115200);

  //generates unique device id
  for(size_t i = 0; i < 9; i++){
    deviceID += UniqueID[i];
  }
  while(deviceID.length() < 24){
    randomSeed(deviceID.toInt());
    deviceID += String(random(0, 9));
  }

  //uses last 6 digits of device id as random seed
  randomSeed(deviceID.substring(18).toInt());
  address = random(8, 120);

  //address 41 is for VL6180
  while(address == 41){
    address = random(8, 120);
  }

  Wire.setClock(400000);
  Wire.begin(address);
  Wire.onReceive(dataReceived);
  Wire.onRequest(dataRequest);

  strip.begin();
  strip.clear();
  strip.show();
}

void loop() {
  if(blink){
    rainbow(5);
    for(int i = 0; i < MAX_PIXELS; i++){
      strip.setPixelColor(i, 0);
    }
    strip.show();
  }
  if(!verifyAddress){
    Serial.print("randomize address: ");
    Serial.println(address);
    address = random(8, 120);
    while(address == 41){
      address = random(8, 120);
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
  strip.show();
}

char c;

void dataReceived(int count){
  Serial.println();
  int size = Wire.available();
  char inputBuffer[size];
  int counter = 0;
  Serial.println();
  Serial.print("address: ");
  Serial.println(address);
  Serial.print("count: ");
  Serial.println(Wire.available());

  Serial.print("data received: ");
  while(Wire.available() > 0){
    c = Wire.read();
    Serial.print(c);
    inputBuffer[counter] = c;
    counter++;
  }
  Serial.println();

  if(size == 1){
    if(inputBuffer[0] == '1'){
      requestMode = 1;
    }else if(inputBuffer[0] == '2'){
      requestMode = 2;
    }else if(inputBuffer[0] == '3'){
      Serial.println("blink on");
      blink = true;
    }else if(inputBuffer[0] == '4'){
      Serial.println("blink off");
      blink = false;
    }
  }else if(size == 24){
    Serial.print("uuid: ");
    Serial.println(deviceID);
    Serial.println("size 24");
    for(int i = 0; i < 24; i++){
      if(deviceID.charAt(i) != inputBuffer[i]){
        Serial.println("BREAK");
        verifyAddress = false;
        break;
      }
    }
  }else if(size == 12){
    Serial.println("size 12");
    char headBuffer[7];
    char tailBuffer[7];
    for(int i = 0; i < 6; i++){
      headBuffer[i] = inputBuffer[i];
      tailBuffer[i] = inputBuffer[i + 6];
    }
    headBuffer[6] = '\0';
    tailBuffer[6] = '\0';
    headColor = strtoul(headBuffer, NULL, 16);
    tailColor = strtoul(tailBuffer, NULL, 16);
    Serial.println(headColor, HEX);
    Serial.println(tailColor, HEX);
  }else{    
    for(int i = 0; i < size + 1; i++){
      if(strip.getPixelColor(i) == headColor || strip.getPixelColor(i) == tailColor){
        strip.setPixelColor(i, 0);
      }
    }
    for(int i = 0; i < size; i++){
      if(inputBuffer[i] == '1'){
        strip.setPixelColor(i - 1, tailColor);
        strip.setPixelColor(i, tailColor);
        strip.setPixelColor(i + 1, headColor);
      }else if(inputBuffer[i] == '5'){
        strip.setPixelColor(i - 1, headColor);
        strip.setPixelColor(i, tailColor);
        strip.setPixelColor(i + 1, tailColor);
      }
    }
  }
}

void dataRequest(){
  Serial.println("request received");
  switch(requestMode){
    case 1:{
      Serial.println("request mode 1");
      Wire.write(deviceID.c_str());
      break;
    }
    case 2:{
      Serial.println("request mode 2");
      Serial.println((unsigned long)UniqueID);
      if(verifyAddress){
        Wire.write("pass");
      }else{
        Wire.write("fail");
      }
      break;
    }
  }
}

void rainbow(uint8_t wait) {
  //if(blink){
    uint16_t i, j;

    for(j=0; j<256; j++) {
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      delay(wait);
    }
  //}
  // else{
  //   for(int i = 0; i < MAX_PIXELS; i++){
  //     strip.setPixelColor(i, 0);
  //   }
  //   strip.show();
  // }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}