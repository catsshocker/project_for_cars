#include "DMA_UART.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <Queue>

DMA_UART uart_cmd(UART_NUM_1,GPIO_NUM_16,GPIO_NUM_17,115200);

QueueHandle_t commendQueue;


void task_commend_getter(void *pvParameters);
void task_respones(void *pvParameters);

void setup(){
    Serial.begin(115200);
    commendQueue = xQueueCreate(1024, sizeof(uint8_t));
    uart_cmd.begin();
    xTaskCreate(task_commend_getter, "task_commend", 2048, NULL, 1, NULL);
    xTaskCreate(task_respones, "task_respones", 2048, NULL, 1, NULL);
    Serial.println("Setup complete");
    pinMode(2,OUTPUT);
}

void loop(){
    
}

void task_commend_getter(void *pvParameters){
    while(1){
        uint8_t data[1024];
        int len = uart_cmd.read_bytes(data, sizeof(data));
        if(len > 0){
            for(int i = 0; i < len; i++){
                xQueueSend(commendQueue, &data[i], portMAX_DELAY);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void task_respones(void *pvParameters){
    const uint8_t start = 0xAA;
    const uint8_t end = 0x55;
    uint8_t pack[256];
    int pack_pos = 0;
    int pack_len = 0;
    int crc = 0;
    enum {WAIT_START,WAIT_LEN, WAIT_DATA,WAIT_CRC, WAIT_END} state = WAIT_START;

    uint8_t data;
    while(1){
        while (xQueueReceive(commendQueue, &data, portMAX_DELAY) == pdTRUE){
            switch (state)
            {
            case WAIT_START:
                if (data == start){
                    pack[pack_pos++] = data;
                    state = WAIT_LEN;
                }
                break;
            
            case WAIT_LEN:
                if (data > 0 && data < 255){
                    pack_len = data;
                    pack[pack_pos++] = data;
                    state = WAIT_DATA;
                }
                else{
                    state = WAIT_START;
                }
                break;
            case WAIT_DATA:
                if (pack_pos < pack_len + 2){
                    pack[pack_pos++] = data;
                }
                if (pack_pos == pack_len + 2){
                    state = WAIT_CRC;
                }
                break;

            case WAIT_CRC:
                if (crc == 0){
                    crc++;
                }
                else{
                    crc = 0;
                    state = WAIT_END;
                }
                break;
            case WAIT_END:
                if (data == end){
                    pack[pack_pos++] = data;
                    Serial.print("Received: ");
                    for (int i = 0; i < pack_pos; i++){
                        Serial.print(pack[i], HEX);
                        Serial.print(" ");
                    }
                    Serial.println();
                }
                state = WAIT_START;
                pack_pos = 0;
                break;

            default:
                break;
            }
        }
    }
}