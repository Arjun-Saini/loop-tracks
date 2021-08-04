/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "e:/IoT/loop-tracks/master/src/master.ino"
#include "HttpClient.h"
#include "JsonParserGeneratorRK.h"

void setup();
void loop();
void randomizeAddress();
#line 4 "e:/IoT/loop-tracks/master/src/master.ino"
const int slaveCountExpected = 3;
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

String redLineStations[] = {"Howard", "Jarvis", "Morse", "Loyola", "Granville", "Thorndale", "Bryn Mawr", "Berwyn", "Argyle", "Lawrence", "Wilson", 
"Sheridan", "Addison", "Belmont", "Fullerton", "North/Clybourn", "Clark/Divison", "Chicago", "Grand", "Lake", "Monroe", "Jackson", "Harrison", 
"Roosevelt", "Cermak-Chinatown", "Sox-35th", "47th", "Garfield", "63rd", "69th", "79th", "87th", "95th/Dan Ryan"};
String redLineOutput[arraySize(redLineStations) * 8];

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

  Wire.begin();
  //randomizeAddress();

  request.hostname = "lapi.transitchicago.com";
  request.port = 80;
  request.path = "/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=red&outputType=JSON";
}

void loop() {
  http.get(request, response, headers);
  // Serial.print("Application>\tResponse status: ");
  // Serial.println(response.status);

  // Serial.print("Application>\tHTTP Response Body: ");
  // Serial.println(response.body);

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
    String trainDir = train.key("trDr").valueString();
    int currentTime = train.key("prdt").valueString().substring(14, 16).toInt();
    int predictedTime = train.key("arrT").valueString().substring(14, 16).toInt();
    int timeDiff = predictedTime -  currentTime;
    //Serial.println(timeDiff);

    if(nextStation.length() <= 1){
      break;
    }
    for(int i = 0; i < arraySize(redLineStations); i++){
      if(nextStation == redLineStations[i]){
        if(trainDir == "1"){
          redLineOutput[i * 8 + timeDiff * 2] = "<";
        }else{
          redLineOutput[i * 8 + 1 + 2 * (3 - timeDiff)] = ">";
        }
      }
    }
    count++;
  }
  for(int i = 0; i < arraySize(redLineOutput); i++){
    if(i % 8 == 0){
      Serial.print("|");
    }
    Serial.print(redLineOutput[i]);
    redLineOutput[i] = "-";
  }
  Serial.println("\n");
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
    Serial.println("\nSequence: ");
    for(int i = 0; i < slaveCountExpected; i++){
      Serial.print(sequenceArr[i]);
      Serial.print(", ");
    }
    //BLE.disconnect();
    //BLE.off();
  }

  bleCount++;
}