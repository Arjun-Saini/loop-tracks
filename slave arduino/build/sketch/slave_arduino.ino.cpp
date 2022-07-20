#line 1 "/Users/sainihome/Documents/GitHub/loop-tracks/slave arduino/slave_arduino.ino"
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
Adafruit_NeoPixel strip(MAX_PIXELS, 2, 0x02);

uint32_t headColor;
uint32_t tailColor;

void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);
unsigned int toInt(char c);
uint32_t stohex(char *input);

#line 27 "/Users/sainihome/Documents/GitHub/loop-tracks/slave arduino/slave_arduino.ino"
void setup();
#line 51 "/Users/sainihome/Documents/GitHub/loop-tracks/slave arduino/slave_arduino.ino"
void loop();
#line 77 "/Users/sainihome/Documents/GitHub/loop-tracks/slave arduino/slave_arduino.ino"
void dataReceived(int count);
#line 184 "/Users/sainihome/Documents/GitHub/loop-tracks/slave arduino/slave_arduino.ino"
void dataRequest();
#line 259 "/Users/sainihome/Documents/GitHub/loop-tracks/slave arduino/slave_arduino.ino"
int x2i(char *s);
#line 27 "/Users/sainihome/Documents/GitHub/loop-tracks/slave arduino/slave_arduino.ino"
void setup() {
    Serial.begin(115200);
    for(size_t i = 0; i < 9; i++){
      deviceID += UniqueID[i];
    }
    while(deviceID.length() < 24){
      deviceID += "0";
    }
  randomSeed(millis());
  address = random(8, 64);
  while(address == 41){
    address = random(8, 64);
  }
  //acquireWireBuffer();
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
    address = random(64, 120);
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
      // for(int i = 0; i < MAX_PIXELS; i++){
      //   strip.setPixelColor(i, 0);
      // }
    }
  }else if(size == 24){
    Serial.print("uuid: ");
    Serial.println(deviceID);
    Serial.println("size 24");
    for(int i = 0; i < 24; i++){
      // Serial.print("deviceID char: ");
      // Serial.print(deviceID.charAt(i));
      // Serial.print(", inputBuffer char: ");
      // Serial.println(inputBuffer[i]);
      if(deviceID.charAt(i) != inputBuffer[i]){
        Serial.println("BREAK");
        verifyAddress = false;
        break;
      }
    }
  }else if(size == 12){
    // String headBuffer = "0x";
    // String tailBuffer = "0x";
    // Serial.println("headBuffer:");
    // for(int i = 0; i < 6; i++){
    //   headBuffer += inputBuffer[i];
    //   Serial.println(headBuffer[i]);
    // }
    // Serial.println("tailBuffer:");
    // for(int i = 6; i < 12; i++){
    //   tailBuffer += inputBuffer[i];
    //   Serial.println(tailBuffer[i]);
    // }
    // Serial.println(headBuffer);
    // Serial.println(tailBuffer);
    // headColor = strtoul(headBuffer.c_str(), NULL, 16);
    // tailColor = strtoul(tailBuffer.c_str(), NULL, 16);
    // Serial.print("headColor: ");
    // Serial.println(headColor, HEX);
    // Serial.print("tailColor: ");
    // Serial.println(tailColor, HEX);

    char *headBuffer = new char[6]();
    char *tailBuffer = new char[6]();
    for(int i = 0; i < 7; i++){
      headBuffer[i] = inputBuffer[i];
      tailBuffer[i] = inputBuffer[i + 6];
    }

    // headColor = strtoul(headBuffer, NULL, 16);
    // tailColor = strtoul(tailBuffer, NULL, 16);
    // sscanf(headBuffer, "%x", &headColor);
    // sscanf(tailBuffer, "%x", &tailColor);
    headColor = x2i(headBuffer);
    tailColor = x2i(tailBuffer);

    // headColor = 0xFFFFFF;
    // tailColor = 0xFFFFFF;
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

uint32_t stohex(char *input){
  uint32_t result = 0U;
  Serial.println("stohex");
  Serial.println(input);
  for(int i = 0; i < 6; i++){
    Serial.print(toInt(input[i]));
    Serial.print(", ");
    result += toInt(input[i]) * pow(16, 5 - i);
  }
  Serial.println(result, HEX);
  return result;
}

unsigned int toInt(char c)
{
  if (c >= '0' && c <= '9') return      c - '0';
  if (c >= 'A' && c <= 'F') return 10 + c - 'A';
  if (c >= 'a' && c <= 'f') return 10 + c - 'a';
  return -1;
}

int x2i(char *s) 
{
  int x = 0;
  for(;;) {
    char c = *s;
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0'; 
    }
    else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10; 
    }else if (c >= 'a' && c <= 'f') {
      x *= 16;
      x += (c - 'a') + 10;
    }else break;
    s++;
  }
  return x;
}
