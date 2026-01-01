#define RXD2 16
#define TXD2 17

int in ;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200,SERIAL_8N1,RXD2,TX2);

}

void loop() {
  while(Serial2.available()){
    Serial.println(Serial2.read());
  }

  delay(5);
}
