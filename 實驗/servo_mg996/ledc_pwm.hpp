#ifndef LEDC_PWM_H
#define LEDC_PWM_H

#include "driver/ledc.h"
#include "Arduino.h"
#include <stdint.h>

class LedcPwm {
private:
    ledc_channel_t channel;
    ledc_timer_t timer;
    int gpioNum;
    int frequency;
    static constexpr int resolution = 16; // 16-bit resolution
public:
    LedcPwm(ledc_channel_t channel, ledc_timer_t timer, int gpioNum, int frequency = 330)
    : channel(channel), timer(timer), gpioNum(gpioNum), frequency(frequency) {
    ledc_timer_config_t timerConfig = {};
    timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    timerConfig.timer_num = timer;
    timerConfig.duty_resolution = LEDC_TIMER_16_BIT;
    timerConfig.freq_hz = frequency;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&timerConfig);

    ledc_channel_config_t channelConfig = {};
    channelConfig.gpio_num = gpioNum;
    channelConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfig.channel = channel;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = timer;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;
    ledc_channel_config(&channelConfig);
    }

    ~LedcPwm() {
        ledc_stop(LEDC_HIGH_SPEED_MODE, channel, 0);
    }

    void setDutyCycle(uint16_t duty) {
        if (duty > (1 << resolution) - 1) duty = (1 << resolution) - 1;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
    }
};

#endif