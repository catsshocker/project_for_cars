#include "DMA_UART.h"

DMA_UART::DMA_UART(uart_port_t uart_port,gpio_num_t rx_pin, gpio_num_t tx_pin,int baud_rate):
_uartPortNum(uart_port),_uartTxPin(tx_pin),_uartRxPin(rx_pin),_uartBaudRate(baud_rate) {
    _uartRxBufSize = 2048;  // Default RX buffer size
    _uartTxBufSize = 2048;  // Default TX buffer size
    
}

DMA_UART::~DMA_UART() {
    uart_driver_delete(_uartPortNum);
}

void DMA_UART::begin() {
    // Configure UART parameters
    uart_config_t uart_config = {
        .baud_rate = _uartBaudRate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    
    // Set UART configuration
    ESP_ERROR_CHECK(uart_driver_install(_uartPortNum, _uartRxBufSize, _uartTxBufSize, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(_uartPortNum, &uart_config));
    
    // Set pins for RX and TX
    ESP_ERROR_CHECK(uart_set_pin(_uartPortNum, _uartTxPin, _uartRxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

}

int DMA_UART::read_bytes(uint8_t *data, uint16_t len) {
    size_t length = len;
    const int result = uart_read_bytes(_uartPortNum, data, length, 100 / portTICK_PERIOD_MS);
    return (result > 0) ? result : 0;
}

void DMA_UART::write_bytes(const uint8_t *data, size_t len) {
    ESP_ERROR_CHECK(uart_write_bytes(_uartPortNum, data, len));
}