#include "hal/uart_types.h"
#include <stdint.h>
#ifndef DMA_UART_H
#define DMA_UART_H

#include "Arduino.h"
#include "driver/uart.h"
#include "esp_log.h"

class DMA_UART {
private:
    uart_port_t _uartPortNum; // UART port number
    int _uartBaudRate;        // UART baud rate
    int _uartRxBufSize = 1024;
    int _uartTxBufSize = 1024;

    gpio_num_t _uartTxPin;
    gpio_num_t _uartRxPin;

public:
    DMA_UART(uart_port_t uart_port,gpio_num_t rx_pin, gpio_num_t tx_pin,int baud_rate);
    ~DMA_UART();
    void begin();
    int read_bytes(uint8_t *data,uint16_t len);
    void write_bytes(const uint8_t *data, size_t len);
};

#endif
