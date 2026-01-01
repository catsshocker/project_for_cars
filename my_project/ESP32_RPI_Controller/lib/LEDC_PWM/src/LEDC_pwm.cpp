#include "LEDC_pwm.h"
#include "esp_log.h"

LEDC_PWM::LEDC_PWM(gpio_num_t ledcPin,ledc_channel_t channel,ledc_timer_t timer, uint32_t freq, ledc_timer_bit_t resolution){  
    _ledcPin = ledcPin;
    _ledcChannel = channel;
    _ledcTimer = timer;
    _ledcFreq = freq;
    _ledcResolution = resolution;

    _max_pwm = pow(2,(int)_ledcResolution)-1;
}

LEDC_PWM::LEDC_PWM(int ledcPin,ledc_channel_t channel,ledc_timer_t timer, uint32_t freq, ledc_timer_bit_t resolution){  
    _ledcPin = gpio_num_t (ledcPin);
    _ledcChannel = channel;
    _ledcTimer = timer;
    _ledcFreq = freq;
    _ledcResolution = resolution;

    _max_pwm = pow(2,(int)_ledcResolution)-1;
}

LEDC_PWM::~LEDC_PWM() {
    ledc_stop( LEDC_HIGH_SPEED_MODE,_ledcChannel, 0); // Stop the LEDC channel
}

void LEDC_PWM::init(){
    ledc_timer_config_t timer_config = {
        .speed_mode      = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = _ledcResolution,
        .timer_num       = _ledcTimer,
        .freq_hz         = _ledcFreq,
        .clk_cfg         = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num   = _ledcPin,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = _ledcChannel,
        .timer_sel  = _ledcTimer,
        .duty       = 0,
        .hpoint     = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

uint32_t LEDC_PWM::setDutyCycle(float dutyCycle){ // dutyCycle 0~100 (%)
    uint32_t i = map_duty(dutyCycle);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE,_ledcChannel,i);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE,_ledcChannel);
    return i;
}

uint32_t LEDC_PWM::map_duty(float dutyCycle){
    return (uint32_t)((dutyCycle/100.0f)*_max_pwm);
}