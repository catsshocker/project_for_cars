#ifndef LEDC_PWM_H
#define LEDC_PWM_H
#include <stdint.h>
#include "hal/ledc_types.h"


#include <Arduino.h>
#include <driver/ledc.h>

class LEDC_PWM
{
private:
    ledc_channel_t _ledcChannel; // LEDC channel
    ledc_timer_t _ledcTimer;     // LEDC timer
    int _ledcFreq;              // Frequency in Hz
    ledc_timer_bit_t _ledcResolution;        // Resolution in bits
    int _ledcDutyCycle;         // Duty cycle (0-100%)
    gpio_num_t _ledcPin;        // GPIO pin for PWM output

    int _max_pwm; //pwm參數最大值(8為解析度256 16位65536)

    uint32_t map_duty(float dutyCycle); // map 0~100 to 0~PWM


public:
    LEDC_PWM(gpio_num_t ledcPin,ledc_channel_t channel,ledc_timer_t timer, uint32_t freq, ledc_timer_bit_t resolution);
    LEDC_PWM(int ledcPin,ledc_channel_t channel,ledc_timer_t timer, uint32_t freq, ledc_timer_bit_t resolution);
    ~LEDC_PWM();
    void init();
    uint32_t setDutyCycle(float dutyCycle); // Set duty cycle (0-100%)
};

#endif
