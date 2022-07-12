/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/sainihome/Documents/GitHub/loop-tracks/master/src/master.ino"
#include "HttpClient.h"
#include "JsonParserGeneratorRK.h"
#include "Railway.cpp"

//all loop scalers must be the same, brown line receives all loop data
void setup();
void loop();
void randomizeAddress();
hal_i2c_config_t acquireWireBuffer();
#line 6 "/Users/sainihome/Documents/GitHub/loop-tracks/master/src/master.ino"
Railway redLine = Railway(
  {Checkpoint(41.853028, -87.63109), Checkpoint(41.9041, -87.628921), Checkpoint(41.903888, -87.639506), Checkpoint(41.913732, -87.652380), Checkpoint(41.9253, -87.65286)},
  {25, 3, 7, 5},
  {0, 40, 0, 0},
  "red",
  {"FF0000", "0A0000"}
);

Railway blueLine = Railway(
  {Checkpoint(41.873647, -87.727931), Checkpoint(41.875666, -87.672961), Checkpoint(41.875293, -87.640976), Checkpoint(41.875660, -87.627620), Checkpoint(41.885738, -87.629540), Checkpoint(41.885698, -87.639828), Checkpoint(41.915497, -87.686258)},
  {12, 8, 5, 5, 5, 25},
  {0, 60, 0, 0},
  "blue",
  {"0000FF", "00000A"}
);

Railway brownLine = Railway(
  {Checkpoint(41.9107586, -87.648068), Checkpoint(41.9103656, -87.6373962), Checkpoint(41.885840, -87.633990), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.885921, -87.626137), Checkpoint(41.885840, -87.633990)},
  {5, 15, 10, 10, 10, 10},
  {20, 0, 40, 0},
  "brown",
  {"FFFF00", "0A0A00"},
  {2, 6, 0, 0}
);

Railway greenLine = Railway(
  {Checkpoint(41.853115, -87.626402), Checkpoint(41.876946, -87.626046), Checkpoint(41.885921, -87.626137), Checkpoint(41.885724, -87.633945), Checkpoint(41.88422, -87.696234)},
  {15, 10, 10, 15},
  {15, 15, 20, 0},
  "green",
  {"00FF00", "000A00"},
  {1, 3, 0, 0}
);

Railway orangeLine = Railway(
  {Checkpoint(41.84678, -87.648088), Checkpoint(41.85817, -87.627117), Checkpoint(41.875689, -87.626019), Checkpoint(41.876955, -87.626044), Checkpoint(41.885921, -87.626137), Checkpoint(41.885840, -87.633990), Checkpoint(41.876835, -87.633710), Checkpoint(41.8767992, -87.6255196)},
  {12, 7, 1, 10, 10, 10, 10},
  {12, 0, 40, 8},
  "orange",
  {"FF8000", "0A0500"},
  {3, 7, 1, 3}
);

Railway purpleLine = Railway(
  {Checkpoint(41.9107586, -87.648068), Checkpoint(41.9103656, -87.6373962), Checkpoint(41.885840, -87.633990), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.885921, -87.626137), Checkpoint(41.885840, -87.633990)},
  {5, 15, 10, 10, 10, 10},
  {20, 0, 40, 0},
  "purple",
  {"2000FF", "02000A"},
  {2, 6, 0, 0}
);

Railway pinkLine = Railway(
  {Checkpoint(41.853964, -87.705408), Checkpoint(41.854856, -87.6695341), Checkpoint(41.8849389, -87.6696133), Checkpoint(41.885840, -87.633990), Checkpoint(41.885921, -87.626137), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.885840, -87.633990)},
  {7, 7, 6, 10, 10, 10, 10},
  {14, 0, 40, 6},
  "pink",
  {"FF8080", "0A0505"},
  {3, 7, 2, 3}
);

std::vector<Railway> railways;

constexpr size_t I2C_BUFFER_SIZE = 512;

int slaveCountExpected = 5; //green adds 2, purple adds 0, every other line adds 1 (7 for full CTA without yellow line)
int brownLineAdr = 0;
int greenLineAdr[2] = {0, 0};

std::vector<int> addressArr = std::vector<int>(slaveCountExpected, 0);
std::vector<int> sequenceArr; //organizes i2c addresses from addressArr
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
  delay(500);

  BLE.on();

  BLE.addCharacteristic(txCharacteristic);
  BLE.addCharacteristic(rxCharacteristic);

  BleAdvertisingData data;
  data.appendServiceUUID(serviceUuid);
  BLE.advertise(&data);

  acquireWireBuffer();
  Wire.begin();

  // request.hostname = "lapi.transitchicago.com";
  // request.port = 80;
  request.hostname = "trek.thewcl.com";
  request.port = 80;

  brownLine.setLoopIndex(2, 6);
  orangeLine.setLoopIndex(3, 7);
  purpleLine.setLoopIndex(2, 6);
  pinkLine.setLoopIndex(3, 7);
  railways = {brownLine, orangeLine, pinkLine, purpleLine, greenLine};

  sequenceArr = std::vector<int>(railways.size() * 2, 0);

  randomizeAddress();
}

void loop(){
  Serial.println("loop start");
  for(int i : sequenceArr){
    Serial.printf("%i, ", i);
  }
  Serial.println();
  Serial.println(brownLineAdr);
  Serial.println(greenLineAdr[0]);
  Serial.println(greenLineAdr[1]);
  
  //while(userInput){
    //loop through each train, loop breaks when all trains have been parsed
    for(int j = 0; j < railways.size(); j++){
      delay(1000);
      // request.path = "/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=" + String(railways[j].name.c_str()) + "&outputType=JSON";
      request.path = "/loop-tracks/cta?lines=" + String(railways[j].name.c_str());
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
        // JsonReference train = parser.getReference().key("ctatt").key("route").index(0).key("train").index(count);
        // String nextStation = train.key("nextStaNm").valueString();
        // String destNm = train.key("destNm").valueString();
        // int trainDir = train.key("trDr").valueString().toInt();
        // float lat = atof(train.key("lat").valueString().c_str());
        // float lon = atof(train.key("lon").valueString().c_str());
        
        //parse json data returned from api
        JsonReference train = parser.getReference().key("lines").index(0).key("trains").index(count);
        String nextStation = train.key("next_stop").valueString();
        String destNm = train.key("destination").valueString();
        int trainDir = train.key("direction").valueInt();
        float lat = train.key("latitude").valueFloat();
        float lon = train.key("longitude").valueFloat();
       
        //break loop once all trains have been parsed
        if(nextStation.length() <= 1){
          Serial.println("break");
          break;
        }
        //Serial.print(String(currentRailway.name.c_str()) + " ");
        //Serial.printf("train %i: ", count);

        int checkpointCount = currentCheckpoints.size();

        //finds index of closest checkpoint to train
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

        //prevents division by zero or extremely large numbers
        if(slope == 0){
          perpendicularSlope = __FLT_MAX__ / 10;
        }else if(slope >= __FLT_MAX__ / 10){
          perpendicularSlope = 0;
        }else{
          perpendicularSlope = -1 / slope;
        }

        bool pointSide, nearestSide, loopPointSide;
        bool validTrain = true;

        //point slope formula to determine which checkpoint is on the other side of the train from the nearest checkpoint
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

        int pcbSegment;
        float segmentPos;
        bool inLoop = false;
        //calculates train position within the segment
        if(validTrain){
          //Serial.printlnf("closestIndex: %i second: %i", closestIndex, secondClosestIndex);
          int lowerIndex, upperIndex;

          if(closestIndex < secondClosestIndex){
            lowerIndex = closestIndex;
            upperIndex = secondClosestIndex;
          }else{
            lowerIndex = secondClosestIndex;
            upperIndex = closestIndex;
          }

          segmentPos = currentRailway.distances[lowerIndex] / (currentRailway.distances[lowerIndex] + currentRailway.distances[upperIndex]);
          segmentPos *= currentRailway.scalers[lowerIndex];

          int lowerScaleBound;
          //in green
          if(closestIndex <= currentRailway.upperGreenBound && closestIndex >= currentRailway.lowerGreenBound && secondClosestIndex <= currentRailway.upperGreenBound && secondClosestIndex >= currentRailway.lowerGreenBound){
            pcbSegment = 3;
            lowerScaleBound = currentRailway.lowerGreenBound;
            Serial.println("in green");
          }
          //before loop
          else if(closestIndex < currentRailway.lowerLoopBound || secondClosestIndex < currentRailway.lowerLoopBound){
            lowerScaleBound = 0;
            pcbSegment = 0;
          }
          //after loop, also for lines that don't go through loop
          else if(closestIndex > currentRailway.upperLoopBound || secondClosestIndex > currentRailway.upperLoopBound){
            lowerScaleBound = currentRailway.upperLoopBound;
            pcbSegment = 1;
          }
          //in loop
          else{
            inLoop = true;
            lowerScaleBound = currentRailway.lowerLoopBound;
            pcbSegment = 2;
          }

          for(int i = lowerScaleBound; i < lowerIndex; i++){
            segmentPos += currentRailway.scalers[i];
          }

          //fix inconsistency from trDr
          // if(currentRailway.name == "purple"){
          //   if(destNm == "Linden"){
          //     trainDir = 1;
          //   }else{
          //     trainDir = 5;
          //   }
          // }
          
          if(inLoop){
            //adjusts output array orientation to match brown line
            if(currentRailway.name == pinkLine.name){
              segmentPos = (float)currentRailway.outputs[2].size() - segmentPos;
            }else if(currentRailway.name == orangeLine.name){
              segmentPos = (int)(segmentPos + orangeLine.outputs[2].size() / 2) % orangeLine.outputs[2].size();
            }
          }else{
            if(currentRailway.name == brownLine.name){
              trainDir = 6 - trainDir;
            }
          }
          if(pcbSegment == 1 && currentRailway.name == greenLine.name){
            segmentPos = (float)currentRailway.outputs[1].size() - segmentPos;
            trainDir = 6 - trainDir;
          }
          if(pcbSegment == 3 && currentRailway.name == pinkLine.name){
            trainDir = 6 - trainDir;
          }
          currentRailway.outputs[pcbSegment][(int)floor(segmentPos)] = trainDir;
        }

        count++;
      }

      //outputs train data to slaves
      for(int i = 0; i < 4; i++){
        //sets color of data being sent
        if(i == 2 || currentRailway.name == purpleLine.name){
          Wire.beginTransmission(brownLineAdr);
        }else if(i == 3){
          if(currentRailway.name == orangeLine.name){
            Wire.beginTransmission(greenLineAdr[0]);
          }else{
            Wire.beginTransmission(greenLineAdr[1]);
          }
        }else{
          Wire.beginTransmission(sequenceArr[j * 2 + i]);
        }

        Wire.write(String(currentRailway.colors[0].c_str()));
        Wire.write(String(currentRailway.colors[1].c_str()));
        Wire.endTransmission();

        if(i == 2 || currentRailway.name == purpleLine.name){
          Wire.beginTransmission(brownLineAdr);
        }else if(i == 3){
          if(currentRailway.name == orangeLine.name){
            Wire.beginTransmission(greenLineAdr[0]);
          }else{
            Wire.beginTransmission(greenLineAdr[1]);
          }
        }else{
          Wire.beginTransmission(sequenceArr[j * 2 + i]);
        }

        if(i == 2){
          //pads green line sending to loop
          if(currentRailway.name == greenLine.name){
            for(int j = 0; j < brownLine.outputs[0].size() + (brownLine.outputs[2].size() / 2); j++){
              Wire.write('0');
            }
          }
          //prevents brown/purple line from getting overriden
          else if(currentRailway.name == brownLine.name || currentRailway.name == purpleLine.name){
            for(int j = 0; j < brownLine.outputs[0].size(); j++){
              Wire.write((char)currentRailway.outputs[0][j] + '0');
            }
          }
          //pads every other line sending to loop
          else{
            for(int j = 0; j < brownLine.outputs[0].size(); j++){
              Wire.write('0');
            }
          }
        }

        //pads blank loop onto the brown/purple track
        if(i == 1 && (currentRailway.name == brownLine.name || currentRailway.name == purpleLine.name)){
          for(int j = 0; j < currentRailway.outputs[2].size(); j++){
            Wire.write('0');
          }
        }

        //pad blank green segment onto orange and pink tracks
        if(i == 3 && currentRailway.name == orangeLine.name){
          for(int j = 0; j < greenLine.outputs[0].size() - orangeLine.outputs[3].size(); j++){
            Wire.write('0');
          }
        }else if(i == 3 && currentRailway.name == pinkLine.name){
          for(int j = 0; j < greenLine.outputs[0].size() - pinkLine.outputs[3].size(); j++){
            Wire.write('0');
          }
        }

        //sends data to slave
        Serial.printf("%s rail part %i: ", currentRailway.name.c_str(), i);
        for(int j = 0; j < currentRailway.outputs[i].size(); j++){
          Wire.write((char)currentRailway.outputs[i][j] + '0');
          Serial.print(currentRailway.outputs[i][j]);
          //currentRailway.outputs[i][j] = 0;
        }
        Serial.println();
        Wire.endTransmission();
      }
      for(int i = 0; i < 4; i++){
        for(int j = 0; j < currentRailway.outputs[i].size(); j++){
          currentRailway.outputs[i][j] = 0;
        }
      }
      Serial.println();
    }
    Serial.println();
  //}
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

      addressArr[count++] = i;
    }
  }

  int seqCount = 0;
  for(int i = 0; i < railways.size(); i++){
    for(int j = 0; j < 2; j++){
      if(railways[i].outputs[j].size() == 0 || railways[i].name == purpleLine.name){
        sequenceArr[2 * i + j] = 0;
      }else{
        sequenceArr[2 * i + j] = addressArr[seqCount++];
        if(railways[i].name == brownLine.name){
          brownLineAdr = sequenceArr[2 * i + j];
        }else if(railways[i].name == greenLine.name){
          greenLineAdr[0] = sequenceArr[2 * i];
          greenLineAdr[1] = sequenceArr[2 * i + 1];
        }
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