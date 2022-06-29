#include "HttpClient.h"
#include "JsonParserGeneratorRK.h"
#include "Railway.cpp"

Railway redLine = Railway(
  LinkedList({Checkpoint(41.853028, -87.63109), Checkpoint(41.9041, -87.628921), Checkpoint(41.903888, -87.639506), Checkpoint(41.913732, -87.652380), Checkpoint(41.9253, -87.65286)}),
  {25, 3, 7, 5},
  40,
  "red",
  {"FF0000", "0A0000"}
);

Railway blueLine = Railway(
  LinkedList({Checkpoint(41.873797, -87.725663), Checkpoint(41.876313, -87.628210), Checkpoint(41.886032, -87.629817), Checkpoint(41.885716, -87.639876), Checkpoint(41.916157, -87.687364)}),
  {25, 5, 5, 25},
  60,
  "blue",
  {"0000FF", "00000A"}
);

// Railway brownLine = Railway(
//   LinkedList({}),
//   {},
//   40,
//   "brown",
//   {"FFFF00", "0A0A00"}
// );

// Railway greenLine= Railway(
//   LinkedList({}),
//   {},
//   40,
//   "green",
//   {"00FF00", "000A00"}
// );

// Railway orangeLine= Railway(
//   LinkedList({}),
//   {},
//   40,
//   "orange",
//   {"FF8000", "0A0500"}
// );

// Railway purpleLine= Railway(
//   LinkedList({}),
//   {},
//   40,
//   "purple",
//   {"800080", "050005"}
// );

// Railway pinkLine= Railway(
//   LinkedList({}),
//   {},
//   40,
//   "pink",
//   {"FF80FF", "0A050A"}
// );

std::vector<Railway> railways = {redLine, blueLine};
//, brownLine, greenLine, orangeLine, purpleLine, pinkLine};

constexpr size_t I2C_BUFFER_SIZE = 512;

const int slaveCountExpected = 2;

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

// Checkpoint redLineCheckpoints[] = {Checkpoint(41.853028, -87.63109), Checkpoint(41.9041, -87.628921), Checkpoint(41.903888, -87.639506), Checkpoint(41.913732, -87.652380), Checkpoint(41.9253, -87.65286)};
// int redLineScalers[] = {25, 3, 7, 5}; //values should add up to RED_NUM_PIXELS
// int redLineOutput[RED_NUM_PIXELS];

HttpClient http;
JsonParserStatic<10000, 1000> parser;

bool userInput = false;

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
}

void loop() {
  Serial.println("loop start");
  
  while(userInput){
    //loop through each train, loop breaks when all trains have been parsed
    for(int j = 0; j < railways.size(); j++){
      delay(1000);
      request.path = strcat(strcat("/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=" , railways.at(j).name.c_str()), "&outputType=JSON");
      http.get(request, response, headers);
      //Serial.println(response.body);

      Serial.println("parsing");
      parser.clear();
      parser.addString(response.body);
      if (!parser.parse()) {
        Serial.println("parsing failed");
        return;
      }

      int count = 0;
      Railway currentRailway = railways.at(j);
      std::vector<Checkpoint> currentCheckpoint = currentRailway.checkpointVector;
      while(true){
        JsonReference train = parser.getReference().key("ctatt").key("route").index(0).key("train").index(count);
        String nextStation = train.key("nextStaNm").valueString();
        Serial.println(nextStation);
        int trainDir = train.key("trDr").valueString().toInt();
        float lat = train.key("lat").valueString().toFloat();
        float lon = train.key("lon").valueString().toFloat();

        if(nextStation.length() <= 1){
          break;
        }

        //Serial.printf("Train %i: ", count);
        int checkpointCount = currentCheckpoint.size();
        for(int i = 0; i < checkpointCount; i++){
          currentRailway.distances.at(i) = currentCheckpoint.at(i).getDistance(lat, lon);
        }
        int closestIndex = std::min_element(currentRailway.distances.begin(), currentRailway.distances.end()) - currentRailway.distances.begin();

        //calculates which checkpoint is on the other side of the train from the nearest checkpoint, works when turns are 90 degrees or less
        float x, x1, y, y1, slope;
        int secondClosestIndex;

        x = lat;
        y = lon;
        x1 = currentCheckpoint.at(closestIndex).lat;
        y1 = currentCheckpoint.at(closestIndex).lon;

        if(currentCheckpoint.at(closestIndex).lat > lat){
          slope = (y1 - y) / (x1 - x);
        }else{
          slope = (y - y1) / (x - x1);
        }

        bool pointSide, nearestSide;
        bool validTrain = true;
        if(closestIndex == 0){
          pointSide = (-1 / slope * (currentCheckpoint.at(closestIndex + 1).lat - x) + y) > currentCheckpoint.at(closestIndex + 1).lon;
          nearestSide = (-1 / slope * (currentCheckpoint.at(closestIndex).lat - x) + y) > currentCheckpoint.at(closestIndex).lon;
          if(pointSide == nearestSide){
            validTrain = false;
          }else{
            secondClosestIndex = 1;
          }
        }else{
          pointSide = (-1 / slope * (currentCheckpoint.at(closestIndex - 1).lat - x) + y) > currentCheckpoint.at(closestIndex - 1).lon;
          nearestSide = (-1 / slope * (currentCheckpoint.at(closestIndex).lat - x) + y) > currentCheckpoint.at(closestIndex).lon;
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
            segmentPos = currentRailway.distances.at(closestIndex) / (currentRailway.distances.at(closestIndex) + currentRailway.distances.at(secondClosestIndex));
            segmentPos *= currentRailway.scalers.at(closestIndex);
            for(int i = 0; i < closestIndex; i++){
              segmentPos += currentRailway.scalers.at(i);
            }
          }else{
            segmentPos = currentRailway.distances.at(secondClosestIndex) / (currentRailway.distances.at(closestIndex) + currentRailway.distances.at(secondClosestIndex));
            segmentPos *= currentRailway.scalers.at(secondClosestIndex);
            for(int i = 0; i < secondClosestIndex; i++){
              segmentPos += currentRailway.scalers.at(i);
            }
          }
          currentRailway.outputs.at((int)floor(segmentPos)) = trainDir;
          //Serial.printlnf("%i, %i, %f", closestIndex, secondClosestIndex, segmentPos);
        }
        count++;
      }
      Wire.beginTransmission(sequenceArr[j]);
      for(int i = 0; i < currentRailway.outputs.size(); i++){
        Wire.write((char)currentRailway.outputs.at(i) + '0');
        currentRailway.outputs.at(i) = 0;
      }
      Wire.endTransmission();
    }
  }

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
    for(int i = 0; i < len - 1; i++){
      inputBuffer += (char)data[i];
      //input = atoi(inputBuffer);
    }

    if(bleCount < slaveCountExpected){
      txCharacteristic.setValue("\nEnter the line color of the device with the blinking LED: ");
    }

    Wire.beginTransmission(addressArr[bleCount]);
    Wire.write('3');
    Wire.endTransmission();

    if(bleCount > 0){
      int colorAdr;
      if(inputBuffer == "red"){
        Serial.println("receive red");
        colorAdr = 0;
      }else if(inputBuffer == "blue"){
        Serial.println("receive blue");
        colorAdr = 1;
      }
      Wire.beginTransmission(addressArr[bleCount - 1]);
      Wire.write(String(railways.at(colorAdr).colors.at(0).c_str()));
      Wire.write(String(railways.at(colorAdr).colors.at(1).c_str()));
      Wire.endTransmission();

      sequenceArr[bleCount - 1] = addressArr[colorAdr];
      
      Wire.beginTransmission(addressArr[bleCount - 1]);
      Wire.write('4');
      Wire.endTransmission();
      if(bleCount == slaveCountExpected){
        userInput = true;
      }
    }
  }

  if(bleCount == slaveCountExpected){
    delay(1000);
    Serial.println("\nSequence: ");
    for(int i = 0; i < slaveCountExpected; i++){
      Serial.print(sequenceArr[i]);
      Serial.print(", ");

      // Wire.beginTransmission(sequenceArr[i]);
      // Wire.write('3');
      // Wire.endTransmission();
      // delay(2000);
      // Wire.beginTransmission(sequenceArr[i]);
      // Wire.write('4');
      // Wire.endTransmission();
    }
    BLE.disconnect();
    BLE.off();
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