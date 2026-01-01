#include <ESP32Servo.h>
#include <ESP32PWM.h>

// #include "ledc_pwm.hpp"
Servo so;

// LedcPwm servo(LEDC_CHANNEL_0,LEDC_TIMER_0,2,50);

void setup() {
  so.attach(2);
}

void loop() {
  so.write(0);
  delay(700);
  so.write(90);
  delay(500);
  so.write(180);
  delay(500);
  // servo.setDutyCycle(65535 * 0.075);
  // delay(1000);

  // // 1ms pulse = 約 5% duty (左邊)
  // servo.setDutyCycle(65535 * 0.05);
  // delay(1000);

  // // 2ms pulse = 約 10% duty (右邊)
  // servo.setDutyCycle(65535 * 0.10);
  // delay(1000);

}
