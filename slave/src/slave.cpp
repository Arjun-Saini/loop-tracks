/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/sainihome/Documents/GitHub/loop-tracks/slave/src/slave.ino"
//#include "dotstar.h"
#include "neopixel.h"

void setup();
void loop();
void dataReceived(int count);
void dataRequest();
hal_i2c_config_t acquireWireBuffer();
#line 4 "/Users/sainihome/Documents/GitHub/loop-tracks/slave/src/slave.ino"
SYSTEM_MODE(MANUAL)

int address;
int requestMode = 0;
bool verifyAddress = false;
String deviceID = System.deviceID();
bool blink = false;
Adafruit_NeoPixel strip(40, D2, 0x02);
constexpr size_t I2C_BUFFER_SIZE = 512;

uint32_t brightRed = 0xFF0000;
uint32_t red = 0x0A0000;

void setup() {
  Serial.begin(9600);
  address = random(8, 64);
  acquireWireBuffer();
  Wire.begin(address);
  Wire.onReceive(dataReceived);
  Wire.onRequest(dataRequest);

  pinMode(D7, OUTPUT);
  strip.begin();
  strip.show();
}

void loop() {
  if(blink){
    digitalWrite(D7, HIGH);
    delay(500);
    digitalWrite(D7, LOW);
    delay(500);
  }

  strip.show();

  delay(100);
}

char c;

void dataReceived(int count){
  int size = Wire.available();
  char inputBuffer[size];
  int counter = 0;

  Serial.print("count: ");
  Serial.println(Wire.available());
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

  if(size == 1 && inputBuffer[0] == '1'){
    requestMode = 1;
  }else if(size == 1 && inputBuffer[0] == '2'){
    requestMode = 2;
  }else if(size == 1 && inputBuffer[0] == '3'){
    blink = true;
  }else if(size == 1 && inputBuffer[0] == '4'){
    blink = false;
  }else if(size == 24){
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
  }else{
    Serial.println("----------------");
    Serial.println(Wire.available());
    for(int i = 0; i < size; i++){
      if(inputBuffer[i] == '0'){
        strip.setPixelColor(i, 0);
      }
    }
    for(int i = 0; i < size; i++){
      if(inputBuffer[i] == '1'){
        strip.setPixelColor(i - 1, red);
        strip.setPixelColor(i, red);
        strip.setPixelColor(i + 1, brightRed);
      }else if(inputBuffer[i] == '5'){
        strip.setPixelColor(i - 1, brightRed);
        strip.setPixelColor(i, red);
        strip.setPixelColor(i + 1, red);
      }
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

hal_i2c_config_t acquireWireBuffer() {
    hal_i2c_config_t config = {
        .size = sizeof(hal_i2c_config_t),
        .version = HAL_I2C_CONFIG_VERSION_1,
        .rx_buffer = new (std::nothrow) uint8_t[I2C_BUFFER_SIZE],
        .rx_buffer_size = I2C_BUFFER_SIZE,
        .tx_buffer = new (std::nothrow) uint8_t[I2C_BUFFER_SIZE],
        .tx_buffer_size = I2C_BUFFER_SIZE
    };
    return config;
}