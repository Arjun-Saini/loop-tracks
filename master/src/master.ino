#include "HttpClient.h"
#include "JsonParserGeneratorRK.h"
#include "Railway.cpp"

Railway redLine = Railway(
  {Checkpoint(41.853028, -87.63109), Checkpoint(41.9041, -87.628921), Checkpoint(41.903888, -87.639506), Checkpoint(41.913732, -87.652380), Checkpoint(41.9253, -87.65286)},
  {25, 3, 7, 5},
  {40, 40, 40},
  "red",
  {"FF0000", "0A0000"}
);

Railway blueLine = Railway(
  {Checkpoint(41.873797, -87.725663), Checkpoint(41.875539, -87.640984), Checkpoint(41.876313, -87.628210), Checkpoint(41.886032, -87.629817), Checkpoint(41.885716, -87.639876), Checkpoint(41.916157, -87.687364)},
  {20, 5, 5, 5, 25},
  {60, 60, 60},
  "blue",
  {"0000FF", "00000A"}
);

Railway brownLine = Railway(
  {Checkpoint(41.885840, -87.633990), Checkpoint(41.885921, -87.626137), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.885840, -87.633990), Checkpoint(41.9103656, -87.6373962), Checkpoint(41.9107586, -87.648068)},
  {5, 5, 5, 5, 15, 5},
  {0, 20, 20},
  "brn",
  {"FFFF00", "0A0A00"},
  {0, 4}
);

Railway greenLine = Railway(
  {Checkpoint(41.853115, -87.626402), Checkpoint(41.885921, -87.626137), Checkpoint(41.88422, -87.696234)},
  {23, 17},
  {40, 40, 40},
  "g",
  {"00FF00", "000A00"}
);

Railway orangeLine = Railway(
  {Checkpoint(41.84678, -87.648088), Checkpoint(41.85817, -87.627117), Checkpoint(41.875689, -87.626019), Checkpoint(41.876955, -87.626044), Checkpoint(41.885921, -87.626137), Checkpoint(41.885840, -87.633990), Checkpoint(41.876835, -87.633710), Checkpoint(41.8767992, -87.6255196)},
  {12, 7, 1, 5, 5, 5, 5},
  {40, 40, 40},
  "org",
  {"FF8000", "0A0500"},
  {3, 7}
);

Railway purpleLine = Railway(
  {Checkpoint(41.885840, -87.633990), Checkpoint(41.885921, -87.626137), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.885840, -87.633990), Checkpoint(41.9103656, -87.6373962), Checkpoint(41.9107586, -87.648068)},
  {5, 5, 5, 5, 15, 5},
  {40, 40, 40},
  "p",
  {"800080", "050005"}
);

Railway pinkLine = Railway(
  {Checkpoint(41.853964, -87.705408), Checkpoint(41.854856, -87.6695341), Checkpoint(41.8849389, -87.6696133), Checkpoint(41.885840, -87.633990), Checkpoint(41.885921, -87.626137), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.885840, -87.633990)},
  {7, 7, 6, 5, 5, 5, 5},
  {40, 40, 40},
  "pink",
  {"FF80FF", "0A050A"},
  {3, 7}
);

std::vector<Railway> railways;

constexpr size_t I2C_BUFFER_SIZE = 512;

const int slaveCountExpected = 2;

int addressArr[2];
int sequenceArr[3];
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

  request.hostname = "lapi.transitchicago.com";
  //request.hostname = "trek.thewcl.com";
  request.port = 80;
  //request.port = 443;

  brownLine.setLoopIndex(4, 0);
  orangeLine.setLoopIndex(3, 7);
  purpleLine.setLoopIndex(4, 0);
  pinkLine.setLoopIndex(3, 7);
  railways = {brownLine};

  randomizeAddress();
}

void loop(){
  Serial.println("loop start");
  
  //while(userInput){
    //loop through each train, loop breaks when all trains have been parsed
    for(int j = 0; j < railways.size(); j++){
      delay(1500);
      request.path = "/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=" + String(railways[j].name.c_str()) + "&outputType=JSON";
      //request.path = "/cta?lines=" + String(railways[j].name.c_str());
      http.get(request, response, headers);

      Serial.println("parsing");
      parser.clear();
      parser.addString(response.body);
      if(!parser.parse()){
        Serial.println("parsing failed");
        return;
      }

      int count = 0;
      Railway currentRailway = railways[j];
      std::vector<Checkpoint> currentCheckpoints = currentRailway.checkpoints;
      while(true){
        JsonReference train = parser.getReference().key("ctatt").key("route").index(0).key("train").index(count);
        String nextStation = train.key("nextStaNm").valueString();
        String destNm = train.key("destNm").valueString();
        int trainDir = train.key("trDr").valueString().toInt();
        float lat = atof(train.key("lat").valueString().c_str());
        float lon = atof(train.key("lon").valueString().c_str());
        
        // JsonReference train = parser.getReference().key("lines").index(0).key("trains").index(count);
        // String nextStation = train.key("next_stop").valueString();
        // String destNm = train.key("destination").valueString();
        // int trainDir = train.key("direction").valueInt();
        // float lat = train.key("latitude").valueFloat();
        // float lon = train.key("longitude").valueFloat();
       
        if(nextStation.length() <= 1){
          Serial.println("break");
          break;
        }
        Serial.print(String(currentRailway.name.c_str()) + " ");
        Serial.printf("train %i: ", count);
        int checkpointCount = currentCheckpoints.size();
        for(int i = 0; i < checkpointCount; i++){
          currentRailway.distances[i] = currentCheckpoints[i].getDistance(lat, lon);
        }
        int closestIndex = std::min_element(currentRailway.distances.begin(), currentRailway.distances.end()) - currentRailway.distances.begin();
        if(closestIndex == currentRailway.tripleIndex){
          closestIndex = currentRailway.loopIndex;
        }

        //calculates which checkpoint is on the other side of the train from the nearest checkpoint, works when turns are 90 degrees or less
        float x, x1, y, y1, slope, perpendicularSlope;
        int secondClosestIndex;

        x = lat;
        y = lon;
        x1 = currentCheckpoints[closestIndex].lat;
        y1 = currentCheckpoints[closestIndex].lon;

        if(currentCheckpoints[closestIndex].lat > lat){
          slope = (y1 - y) / (x1 - x);
        }else{
          slope = (y - y1) / (x - x1);
        }

        if(slope == 0){
          perpendicularSlope = __FLT_MAX__ / 10;
          Serial.println("slope 0");
        }else if(slope >= __FLT_MAX__ / 10){
          perpendicularSlope = 0;
          Serial.println("slope max");
        }else{
          perpendicularSlope = -1 / slope;
        }

        bool pointSide, nearestSide, loopPointSide;
        bool validTrain = true;

        Serial.println("calculation");
        if(closestIndex == 0){
          pointSide = (perpendicularSlope * (currentCheckpoints[closestIndex + 1].lat - x) + y) > currentCheckpoints[closestIndex + 1].lon;
          nearestSide = (perpendicularSlope * (currentCheckpoints[closestIndex].lat - x) + y) > currentCheckpoints[closestIndex].lon;
          if(currentRailway.loopIndex == closestIndex){
            loopPointSide = (perpendicularSlope * (currentCheckpoints[currentRailway.tripleIndex - 1].lat - x) + y) > currentCheckpoints[currentRailway.tripleIndex - 1].lon;
            if(nearestSide != pointSide){
              secondClosestIndex = 1;
            }else if(nearestSide != loopPointSide){
              closestIndex = currentRailway.tripleIndex;
              secondClosestIndex = currentRailway.tripleIndex - 1;
            }else{
              closestIndex = currentRailway.tripleIndex;
              secondClosestIndex = currentRailway.tripleIndex + 1;
            }
          }else{
            if(pointSide == nearestSide){
              validTrain = false;
            }else{
              secondClosestIndex = 1;
            }
          }
        }else{
          pointSide = (perpendicularSlope * (currentCheckpoints[closestIndex - 1].lat - x) + y) > currentCheckpoints[closestIndex - 1].lon;
          nearestSide = (perpendicularSlope * (currentCheckpoints[closestIndex].lat - x) + y) > currentCheckpoints[closestIndex].lon;
          if(closestIndex == checkpointCount - 1){
            if(closestIndex == currentRailway.loopIndex){
              loopPointSide = (perpendicularSlope * (currentCheckpoints[currentRailway.tripleIndex - 1].lat - x) + y) > currentCheckpoints[currentRailway.tripleIndex - 1].lon;
              if(nearestSide != pointSide){
                secondClosestIndex = checkpointCount - 2;
              }else if(nearestSide != loopPointSide){
                closestIndex = currentRailway.tripleIndex;
                secondClosestIndex = currentRailway.tripleIndex - 1;
              }else{
                closestIndex = currentRailway.tripleIndex;
                secondClosestIndex = currentRailway.tripleIndex + 1;
              }
            }else{
              if(pointSide == nearestSide){
                validTrain = false;
              }else{
                secondClosestIndex = checkpointCount - 2;
              }
            }
          }else{
            if(pointSide == nearestSide){
              secondClosestIndex = closestIndex + 1;
            }else{
              secondClosestIndex = closestIndex - 1;
            }
          }
        }

        Serial.println("segment pos");
        int pcbSegment;
        float segmentPos;
        if(validTrain){
          int lowerIndex, upperIndex;

          if(closestIndex < secondClosestIndex){
            lowerIndex = closestIndex;
            upperIndex = secondClosestIndex;
          }else{
            lowerIndex = secondClosestIndex;
            upperIndex = closestIndex;
          }

          int lowerScaleBound;
          //before loop
          if(closestIndex < currentRailway.lowerLoopBound || secondClosestIndex < currentRailway.lowerLoopBound){
            lowerScaleBound = 0;
            pcbSegment = 0;
            Serial.println("before");
          }
          //after loop
          else if(closestIndex > currentRailway.upperLoopBound || secondClosestIndex > currentRailway.upperLoopBound){
            lowerScaleBound = currentRailway.upperLoopBound;
            pcbSegment = 1;
            Serial.println("after");
          }
          //in loop
          else{
            lowerScaleBound = currentRailway.lowerLoopBound;
            pcbSegment = 2;
            Serial.println("in");

            if(currentRailway.name == "brn" || currentRailway.name == "pink"){
               trainDir = 6 - trainDir;
            }
          }
          
          segmentPos = currentRailway.distances[lowerIndex] / (currentRailway.distances[lowerIndex] + currentRailway.distances[upperIndex]);

          Serial.printlnf("lower scale bound: %i, lowerIndex: %i, initial segpos %f, scalers: ", lowerScaleBound, lowerIndex, segmentPos);

          segmentPos *= currentRailway.scalers[lowerIndex];

          for(int i = lowerScaleBound; i < lowerIndex; i++){
            segmentPos += currentRailway.scalers[i];
            Serial.printf("%i, ", currentRailway.scalers[i]);
          }

          //fix inconsistency from trDr
          if(currentRailway.name == "p"){
            if(destNm == "Linden"){
              trainDir = 1;
            }else{
              trainDir = 5;
            }
          }
          Serial.println("output vector");
          currentRailway.outputs[pcbSegment][(int)floor(segmentPos)] = trainDir;
          Serial.printlnf("%i, %i, %f", closestIndex, secondClosestIndex, segmentPos);
        }

        count++;
      }
      for(int i = 0; i < 3; i++){
        Serial.println("sending");
        Wire.beginTransmission(sequenceArr[j * 3 + i]);
        Wire.write(String(currentRailway.colors[0].c_str()));
        Wire.write(String(currentRailway.colors[1].c_str()));
        Wire.endTransmission();

        Wire.beginTransmission(sequenceArr[j * 3 + i]);
        Serial.printlnf("pcb segment %i", i);
        for(int j = 0; j < currentRailway.outputs[i].size(); j++){
          Wire.write((char)currentRailway.outputs[i][j] + '0');
          Serial.print(currentRailway.outputs[i][j]);
          currentRailway.outputs[i][j] = 0;
        }
        Wire.endTransmission();
      }

      Serial.println();
    }
    Serial.println();
  //}
  Serial.println();
  delay(500);
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

  int seqCount = 0;
  for(int i = 0; i < railways.size(); i++){
    for(int j = 0; j < 3; j++){
      if(railways[i].outputs[j].size() == 0){
        sequenceArr[3 * i + j] = 0;
      }else{
        sequenceArr[3 * i + j] = addressArr[seqCount++];
      }
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
      int colorAdr = 0;
      if(inputBuffer == "red"){
        Serial.println("receive red");
        colorAdr = 0;
      }else if(inputBuffer == "blue"){
        Serial.println("receive blue");
        colorAdr = 1;
      }else if(inputBuffer == "green"){
        Serial.println("receive green");
        colorAdr = 2;
      }
      // Wire.beginTransmission(addressArr[bleCount - 1]);
      // Wire.write(String(railways.at(colorAdr).colors.at(0).c_str()));
      // Wire.write(String(railways.at(colorAdr).colors.at(1).c_str()));
      // Wire.endTransmission();

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
    // BLE.disconnect();
    // BLE.off();
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