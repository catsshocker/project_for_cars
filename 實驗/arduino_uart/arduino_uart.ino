#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);  // RX=D2, TX=D3

void setup() {
    mySerial.begin(9600);   // 設定與 ESP32 通訊的 UART
}

void loop() {
    if (mySerial.available()) {
        String data = mySerial.readString(); // 讀取 ESP32 傳來0的數據
        mySerial.println("Arduino 收到!");  // 回應 ESP32
    }
}
