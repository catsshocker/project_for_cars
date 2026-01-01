#include "EncoderPCNT.h"

EncoderPCNT encoder(GPIO_NUM_36,GPIO_NUM_34,PCNT_UNIT_0);


void setup() {
  encoder.begin();
  Serial.begin(115200);
}

void loop() {
  Serial.println(encoder.getCount());
  delay(300);
}
