#include "HttpClient.h"
#include "JsonParserGeneratorRK.h"
#include "math.h"
#include "dotstar.h"

#define LINE_COUNT 2
#define RED_LINE_VERTICAL 55
#define RED_LINE_HORIZONTAL 17
#define BLUE_LINE_UPPER_HORIZONTAL 33
#define BLUE_LINE_VERTICAL 16
#define BLUE_LINE_LOWER_HORIZONTAL 23

constexpr size_t I2C_BUFFER_SIZE = 512;

const int slaveCountExpected = 1;
int addressArr[slaveCountExpected];
int sequenceArr[slaveCountExpected];
int slaveCount, bleCount;

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);

const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

BleCharacteristic txCharacteristic("tx", BleCharacteristicProperty::NOTIFY, txUuid, serviceUuid);
BleCharacteristic rxCharacteristic("rx", BleCharacteristicProperty::WRITE_WO_RSP, rxUuid, serviceUuid, onDataReceived, NULL);

http_header_t headers[] = {
  {"Accept", "/*/"},
  {NULL, NULL}
};

http_request_t request;
http_response_t response;

int redLineOutput[RED_LINE_VERTICAL + RED_LINE_HORIZONTAL];
int blueLineOutput[BLUE_LINE_VERTICAL + BLUE_LINE_UPPER_HORIZONTAL + BLUE_LINE_LOWER_HORIZONTAL];

HttpClient http;
JsonParserStatic<10000, 1000> parser;

void setup() {
  Serial.begin(9600);
  delay(2000);
  BLE.on();

  BLE.addCharacteristic(txCharacteristic);
  BLE.addCharacteristic(rxCharacteristic);

  BleAdvertisingData data;
  data.appendServiceUUID(serviceUuid);
  BLE.advertise(&data);

  acquireWireBuffer();
  Wire.begin();
  randomizeAddress();

  request.hostname = "lapi.transitchicago.com";
  request.port = 80;
  request.path = "/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=red&outputType=JSON";
}

uint32_t brightRed = 0xFF0000;
uint32_t red = 0x0A0000;

void loop() {
  for(int i = 0; i < LINE_COUNT; i++){
    // switch(i){
    //   case 0:{
    //     request.path = "/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=red&outputType=JSON";
    //     break;
    //   }
    //   case 1:{
    //     request.path = "/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=blue&outputType=JSON";
    //     break;
    //   }
    // }
    response.body = "";
    http.get(request, response, headers);
    Serial.println("------------------------------------------------------------");
    Serial.println(response.body);

    parser.clear();
    parser.addString(response.body);
    if (!parser.parse()) {
      Serial.println("parsing failed on: ");
      Serial.println(response.body);
    }
    
    //loop through each train, loop breaks when all trains have been parsed
    int count = 0;
    bool validTrain = false;
    while(true){
      JsonReference train = parser.getReference().key("ctatt").key("route").index(0).key("train").index(count);
      String nextStation = train.key("nextStaNm").valueString();
      String trainDir = train.key("trDr").valueString();
      float lat = train.key("lat").valueString().toFloat();
      float lon = train.key("lon").valueString().toFloat();
      int pos;

      if(nextStation.length() <= 1){
        break;
      }

      switch(i){
        case 0:{
          if(lat < 41.89950 && lat > 41.853206){
            pos = (int) (RED_LINE_VERTICAL * (lat - 41.853206) / (41.89950 - 41.853206) + 0.5);
            validTrain = true;
          }else if(lat > 41.89950 && lon < -87.628176 && lat < 41.910655){
            pos = (int) (RED_LINE_HORIZONTAL * (lon - -87.628176) / (-87.649177 - -87.628176) + 0.5) + RED_LINE_VERTICAL;
            validTrain = true;
          }
          if(trainDir == "1" && validTrain){
            redLineOutput[pos] = 1;
            Serial.println(pos);
          }else if(trainDir == "5" && validTrain){
            redLineOutput[pos] = 5;
            Serial.println(pos);
          }
          break;
        }
        case 1:{
          if(lat < 41.878183 && lon < -87.631722 && lon > -87.725663){
            pos = (int) (BLUE_LINE_LOWER_HORIZONTAL * (lon - -87.725663) / (-87.631722 - -87.725663) + 0.5);
            validTrain = true;
          }else if(lon > -87.631722 && lat < 41.885737 && lat > 41.875568){
            pos = (int) (BLUE_LINE_VERTICAL * (lat - 41.875568) / (41.885737 - 41.875568) + 0.5) + BLUE_LINE_LOWER_HORIZONTAL;
            validTrain = true;
          }else if(lat > 41.883164 && lon < -87.630886 && lon > -87.677437){
            pos = (int) (BLUE_LINE_UPPER_HORIZONTAL * (lon - -87.630886) / (-87.677437 - -87.630886) + 0.5) + BLUE_LINE_LOWER_HORIZONTAL + BLUE_LINE_VERTICAL;
            validTrain = true;
          }
          if(trainDir == "1" && validTrain){
            blueLineOutput[pos] = 1;
          }else if(trainDir == "5" && validTrain){
            blueLineOutput[pos] = 5;
          }
          break;
        }
      }

      validTrain = false;
      count++;
    }

    Serial.println();
    switch(i){
      case 0:{
        Wire.beginTransmission(addressArr[0]);
        for(int i = 0; i < arraySize(redLineOutput); i++){
          Wire.write(char(redLineOutput[i] + '0'));
          redLineOutput[i] = 0;
        }
        Wire.endTransmission();
        break;
      }
    }
  }
  delay(5000);
}

//clears up conflicts with multiple i2c slaves having the same address
void randomizeAddress(){
  while(slaveCount != slaveCountExpected){
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

  Serial.println("\nConnected to: ");

  int count = 0;
  for(int i = 8; i<= 119; i++){
    Wire.beginTransmission(i);
    Wire.write('1');
    Wire.endTransmission();

    Wire.requestFrom(i, 24);
    if(Wire.available() > 0){
      Serial.print(i);
      Serial.print(", ");

      addressArr[count] = i;
      count++;
    }
  }
}

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context){
  String inputBuffer = "";

  if(bleCount <= slaveCountExpected){
    int input;

    for(int i = 0; i < len - 1; i++){
      inputBuffer += (char)data[i];
      input = atoi(inputBuffer);
    }

    if(bleCount < slaveCountExpected){
      txCharacteristic.setValue("\nEnter the position of the device with the blue LED as an integer (first device is at 1, second is at 2, etc): ");
    }

    Wire.beginTransmission(addressArr[bleCount]);
    Wire.write('3');
    Wire.endTransmission();

    if(bleCount > 0){
      sequenceArr[input - 1] = addressArr[bleCount - 1];
      
      Wire.beginTransmission(addressArr[bleCount - 1]);
      Wire.write('4');
      Wire.endTransmission();
    }
  }

  if(bleCount == slaveCountExpected){
    delay(1000);
    Serial.println("\nSequence: ");
    for(int i = 0; i < slaveCountExpected; i++){
      Serial.print(sequenceArr[i]);
      Serial.print(", ");

      Wire.beginTransmission(sequenceArr[i]);
      Wire.write('3');
      Wire.endTransmission();
      delay(2000);
      Wire.beginTransmission(sequenceArr[i]);
      Wire.write('4');
      Wire.endTransmission();
    }
    //BLE.disconnect();
    //BLE.off();
  }

  bleCount++;
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