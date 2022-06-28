#include "HttpClient.h"
#include "JsonParserGeneratorRK.h"
#include "math.h"
#include "neopixel.h"

#define RED_NUM_PIXELS 40

//checkpoints mark each turn on a line
class Checkpoint{
  public:
  
    float lat;
    float lon;

    Checkpoint(float la, float lo){
      lat = la;
      lon = lo;
    }

    float getDistance(float trainLat, float trainLon){
      return sqrt(pow((trainLat - lat), 2) + pow((trainLon - lon), 2));
    }
};

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

Checkpoint redLineCheckpoints[] = {Checkpoint(41.853028, -87.63109), Checkpoint(41.9041, -87.628921), Checkpoint(41.903888, -87.639506), Checkpoint(41.913732, -87.652380), Checkpoint(41.9253, -87.65286)};
int redLineScalers[] = {25, 3, 7, 5}; //values should add up to RED_NUM_PIXELS
int redLineOutput[RED_NUM_PIXELS];

HttpClient http;
JsonParserStatic<10000, 1000> parser;

Adafruit_NeoPixel strip(RED_NUM_PIXELS, D2, 0x02);

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
  //randomizeAddress();

  request.hostname = "lapi.transitchicago.com";
  request.port = 80;
  request.path = "/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=red&outputType=JSON";

  strip.begin();
  strip.show();
}

void loop() {
  http.get(request, response, headers);

  parser.clear();
	parser.addString(response.body);
  if (!parser.parse()) {
		Serial.println("parsing failed");
		return;
	}
  
  //loop through each train, loop breaks when all trains have been parsed
  int count = 0;
  while(true){
    JsonReference train = parser.getReference().key("ctatt").key("route").index(0).key("train").index(count);
    String nextStation = train.key("nextStaNm").valueString();
    int trainDir = train.key("trDr").valueString().toInt();
    float lat = train.key("lat").valueString().toFloat();
    float lon = train.key("lon").valueString().toFloat();

    if(nextStation.length() <= 1){
      break;
    }

    //Serial.printf("Train %i: ", count);
    int checkpointCount = sizeof(redLineCheckpoints) / sizeof(redLineCheckpoints[0]);
    float checkpointDistances[checkpointCount];
    for(int i = 0; i < checkpointCount; i++){
      checkpointDistances[i] = redLineCheckpoints[i].getDistance(lat, lon);
    }
    float* closestCheckpoint = std::min_element(checkpointDistances, checkpointDistances + checkpointCount);
    int closestIndex = closestCheckpoint - checkpointDistances;

    //calculates which checkpoint is on the other side of the train from the nearest checkpoint, works when turns are 90 degrees or less
    float x, x1, y, y1, slope;
    int secondClosestIndex;

    x = lat;
    y = lon;
    x1 = redLineCheckpoints[closestIndex].lat;
    y1 = redLineCheckpoints[closestIndex].lon;

    if(redLineCheckpoints[closestIndex].lat > lat){
      slope = (y1 - y) / (x1 - x);
    }else{
      slope = (y - y1) / (x - x1);
    }

    bool pointSide, nearestSide;
    bool validTrain = true;
    if(closestIndex == 0){
      pointSide = (-1 / slope * (redLineCheckpoints[closestIndex + 1].lat - x) + y) > redLineCheckpoints[closestIndex + 1].lon;
      nearestSide = (-1 / slope * (redLineCheckpoints[closestIndex].lat - x) + y) > redLineCheckpoints[closestIndex].lon;
      if(pointSide == nearestSide){
        validTrain = false;
      }else{
        secondClosestIndex = 1;
      }
    }else{
      pointSide = (-1 / slope * (redLineCheckpoints[closestIndex - 1].lat - x) + y) > redLineCheckpoints[closestIndex - 1].lon;
      nearestSide = (-1 / slope * (redLineCheckpoints[closestIndex].lat - x) + y) > redLineCheckpoints[closestIndex].lon;
      if(closestIndex == checkpointCount - 1){
        if(pointSide == nearestSide){
          validTrain = false;
        }else{
          secondClosestIndex = checkpointCount - 2;
        }
      }else{
        if(pointSide == nearestSide){
          secondClosestIndex = closestIndex + 1;
        }else{
          secondClosestIndex = closestIndex - 1;
        }
      }
    }

    float segmentPos;
    if(validTrain){
      if(closestIndex < secondClosestIndex){
        segmentPos = checkpointDistances[closestIndex] / (checkpointDistances[closestIndex] + checkpointDistances[secondClosestIndex]);
        segmentPos *= redLineScalers[closestIndex];
        for(int i = 0; i < closestIndex; i++){
          segmentPos += redLineScalers[i];
        }
      }else{
        segmentPos = checkpointDistances[secondClosestIndex] / (checkpointDistances[closestIndex] + checkpointDistances[secondClosestIndex]);
        segmentPos *= redLineScalers[secondClosestIndex];
        for(int i = 0; i < secondClosestIndex; i++){
          segmentPos += redLineScalers[i];
        }
      }
      redLineOutput[(int)floor(segmentPos)] = trainDir;
      //Serial.printlnf("%i, %i, %f", closestIndex, secondClosestIndex, segmentPos);
    }

    count++;
  }
  for(int i = 0; i < RED_NUM_PIXELS; i++){
    Serial.print(redLineOutput[i]);
    strip.setPixelColor(i, 0);
    if(redLineOutput[i] == 1){
      strip.setPixelColor(i, 0xFF0000);
    }else if(redLineOutput[i] == 5){
      strip.setPixelColor(i, 0x0000FF);
    }
    redLineOutput[i] = 0;
  }
  strip.show();
  Serial.println();

  // for(int i = 0; i < arraySize(redLineOutput); i++){
  //   if(redLineOutput[i] == 1){
  //     strip.setPixelColor(i - 1, red);
  //     strip.setPixelColor(i, red);
  //     strip.setPixelColor(i + 1, brightRed);
  //   }else if(redLineOutput[i] == 5){
  //     strip.setPixelColor(i - 1, brightRed);
  //     strip.setPixelColor(i, red);
  //     strip.setPixelColor(i + 1, red);
  //   }
  //   redLineOutput[i] = 0;
  // }

  //strip.show();
  delay(5000);
  Serial.println();
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