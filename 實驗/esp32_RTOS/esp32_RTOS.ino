#include <Arduino.h>

int delayTime = 500;


void Task1(void *pvParameters) {
  while (1) {
    digitalWrite(2, !digitalRead(2));
    // Serial.println("led is change");
    vTaskDelay(delayTime / portTICK_PERIOD_MS);
  }
}

void Task2(void *pvParameters) {
  while (1) {
    if(Serial.available()){
      delayTime = Serial.readString().toInt();
      Serial.printf("Set delay %d ms\n",delayTime);
      vTaskDelay(200/portTICK_PERIOD_MS);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  xTaskCreatePinnedToCore(
    Task1, "Task1", 2048, NULL, 1, NULL, 0);

  xTaskCreatePinnedToCore(
    Task2, "Task2", 2048, NULL, 2, NULL, 1);
}

void loop() {
  // put your main code here, to run repeatedly:
}
