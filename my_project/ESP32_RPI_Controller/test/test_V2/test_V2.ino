#include "DMA_UART.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <Queue>

DMA_UART uart_cmd(UART_NUM_1,GPIO_NUM_16,GPIO_NUM_17,115200);

QueueHandle_t commendQueue;

QueueHandle_t IoControlQueue;



struct Command{
    uint8_t cmd_id;
    uint8_t len;
    uint8_t data[255];
};

void task_commend_getter(void *pvParameters);
void task_command_dispatcher(void *pvParameters);
void task_IOControl(void *pvParameters);

void setup(){
    Serial.begin(115200);

    commendQueue = xQueueCreate(1024, sizeof(uint8_t));
    IoControlQueue = xQueueCreate(1024, sizeof(Command));

    uart_cmd.begin();

    xTaskCreate(task_commend_getter, "task_commend", 2048, NULL, 1, NULL);
    xTaskCreate(task_command_dispatcher, "task_respones", 2048, NULL, 1, NULL);

    Serial.println("Setup complete");
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

// 建議命名為 task_command_dispatcher 或 task_packet_parser
void task_command_dispatcher(void *pvParameters){
    const uint8_t start = 0xAA;
    const uint8_t end = 0x55;
    uint8_t pack[256];
    int pack_pos = 0;
    int pack_len = 0;
    int crc = 0;
    enum {WAIT_START,WAIT_LEN, WAIT_DATA,WAIT_CRC, WAIT_END} state = WAIT_START;

    Command cmd;
    uint8_t cmd_len = 0;

    uint8_t inputByte;
    while(1){
        while (xQueueReceive(commendQueue, &inputByte, portMAX_DELAY) == pdTRUE){
            switch (state)
            {
            case WAIT_START:
                if (inputByte == start){
                    pack[pack_pos++] = inputByte;
                    // 初始化指令物件
                    memset(&cmd, 0, sizeof(Command));
                    cmd_len = 0;
                    state = WAIT_LEN;
                }
                break;

            case WAIT_LEN:
                if (inputByte > 0 && inputByte < 255){
                    pack_len = inputByte;
                    pack[pack_pos++] = inputByte;
                    cmd.len = inputByte;
                    state = WAIT_DATA;
                }
                else{
                    state = WAIT_START;
                    pack_pos = 0;
                }
                break;

            case WAIT_DATA:
                if (pack_pos < pack_len + 2){
                    pack[pack_pos++] = inputByte;
                    if(cmd_len == 0) {
                        cmd.cmd_id = inputByte;
                    }
                    cmd.data[cmd_len ++] = inputByte;
                }
                if (pack_pos == pack_len + 2){
                    state = WAIT_CRC;
                }
                break;

            case WAIT_CRC:
                // 這裡應該要計算CRC，暫時略過
                state = WAIT_END;
                break;

            case WAIT_END:
                if (inputByte == end){
                    pack[pack_pos++] = inputByte;
                    Serial.print("Received: ");
                    for (int i = 0; i < pack_pos; i++){
                        Serial.print(pack[i], HEX);
                        Serial.print(" ");
                    }
                    Serial.println();

                    switch (cmd.cmd_id >> 4)
                    {
                    case 0x00: // Command ID 0x00(General command)
                        // Send the command to the commendQueue
                        xQueueSend(commendQueue, &cmd, portMAX_DELAY);
                        break;
                    
                    case 0x01: // Command ID 0x01(IO command)
                        // Send the command to the IoControlQueue
                        xQueueSend(IoControlQueue, &cmd, portMAX_DELAY);
                        break;
                    
                    default:
                        break;
                    }
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

void task_IOControl(void *pvParameters){
    Command cmd;
    while(1){
        if(xQueueReceive(IoControlQueue, &cmd, portMAX_DELAY) == pdTRUE){
            // Process the command
            Serial.print("Command ID: ");
            Serial.println(cmd.cmd_id);
            Serial.print("Command Length: ");
            Serial.println(cmd.len);
            Serial.print("Command Data: ");
            for(int i = 0; i < cmd.len; i++){
                Serial.print(cmd.data[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
    }
}