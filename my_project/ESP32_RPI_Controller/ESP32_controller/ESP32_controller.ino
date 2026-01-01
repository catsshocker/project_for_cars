#include "DMA_UART.h"
#include <arduino.h>

DMA_UART uart01(UART_NUM_1, GPIO_NUM_16, GPIO_NUM_17, 115200);

void setup() { 
    uart01.begin();
    Serial.begin(921600);
 }

void loop() {
    uint8_t buffer[200] = {};
    int len = uart01.read_bytes(buffer, sizeof(buffer));

    for (int i = 0; i < len; i++) {
        Serial.print(buffer[i],HEX);
        Serial.print(" ");
    }
    Serial.println();


    delay(500);
}
