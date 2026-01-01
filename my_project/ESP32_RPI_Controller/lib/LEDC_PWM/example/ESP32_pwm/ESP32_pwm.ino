#include "LEDC_pwm.h"

const int LED_PIN = 2;

LEDC_PWM pwm1(LED_PIN,LEDC_CHANNEL_0,LEDC_TIMER_1,50,ledc_timer_bit_t(8));

void setup(){
    Serial.begin(115200);
    pinMode(2,OUTPUT);
    pwm1.init();
}

void loop(){
    for(int i=0;i<=100;i++){
        Serial.println(pwm1.setDutyCycle(i));
        delay(10);
    }
    for(int i=100;i>=0;i--){
        Serial.println(pwm1.setDutyCycle(i));
        delay(10);
    }
}