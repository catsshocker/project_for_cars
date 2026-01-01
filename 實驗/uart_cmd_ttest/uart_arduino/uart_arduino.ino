void setup() {
  Serial.begin(115200);

  delay(100);

  for(int i = 0;i<128;i++){
    Serial.write((uint8_t)i);
  }
  uint8_t msg[5] = {0x11,0x22,0x33,0x44,0x55};

  Serial.write(msg,5);

}

void loop() {
  

}
