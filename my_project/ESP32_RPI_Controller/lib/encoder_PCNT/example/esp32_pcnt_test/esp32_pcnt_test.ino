#include "EncoderPCNT.h"

EncoderPCNT encoder(GPIO_NUM_36, GPIO_NUM_34, PCNT_UNIT_0);


void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_DEBUG);  // 設置全域日誌等級
  encoder.begin();
}

void loop() {
  int32_t count = encoder.get_count();
  Serial.println(count);
  delay(100);
}
