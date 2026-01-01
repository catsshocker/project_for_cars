#include <HardwareSerial.h>

HardwareSerial MySerial{ 2 };

void setup() {
  Serial.begin(115200);
  MySerial.begin(9600, SERIAL_8N1, 16, 17);
}

void loop() {
  MySerial.println("Hello Arduino!");
  Serial.println("已發送: Hello Arduino!");

  delay(800);

  if (MySerial.available()) {
    String response = MySerial.readString();  // 讀取 Arduino 回應
    Serial.print("來自 Arduino: ");
    Serial.println(response);
  }

  delay(1000);
}
