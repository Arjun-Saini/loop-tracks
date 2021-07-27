SYSTEM_MODE(MANUAL)

int slaveCountExpected = 1;
int slaveCount;

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  Wire.beginTransmission(0x20);
  Wire.write("transmission");
  Wire.endTransmission();

  Wire.requestFrom(0x20, 7);
  Serial.println(Wire.available());
  delay(500);
}

void randomizeAddress(){
  byte transmission;
  while(slaveCount != slaveCountExpected){
    //scan through each i2c address
    slaveCount = 0;
    for(int i = 0x08; i <= 0x77; i++){
      Wire.requestFrom(i, 1);
      if(Wire.available()){
        transmission = Wire.read();
        if(i == transmission){
          slaveCount++;
        }
      }
    }

    //randomize addresses on conflict
    if(slaveCount != slaveCountExpected){

    }
  }
}