SYSTEM_MODE(MANUAL)

int address = 0x20;

void setup() {
  Serial.begin(9600);
  Wire.begin(address);
  Wire.onReceive(dataReceived);
  Wire.onRequest(dataRequest);
}

void loop() {
  Serial.println(Wire.available());
  delay(100);
}

void dataReceived(int count){
  Serial.println("data received");
}

void dataRequest(){
  Serial.println("request received");
  Wire.write(address);
}