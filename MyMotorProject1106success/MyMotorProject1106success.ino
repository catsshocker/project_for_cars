#include "EncoderPCNT.h"
#include <Arduino.h>

// === TB6612FNG 馬達腳位設定 ===
#define STBY 26
// 左馬達
#define AIN1 25
#define AIN2 33
#define PWMA 32

// 右馬達
#define BIN1 27
#define BIN2 14
#define PWMB 12

// === 編碼器腳位設定 ===
EncoderPCNT leftEncoder(GPIO_NUM_35, GPIO_NUM_34, PCNT_UNIT_0);
EncoderPCNT rightEncoder(GPIO_NUM_39, GPIO_NUM_36, PCNT_UNIT_1);

// === 馬達控制函式 ===
void motorStop() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
  ledcWrite(0, 0);
  ledcWrite(1, 0);
}

void motorForward(int speed) {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  ledcWrite(0, speed);
  ledcWrite(1, speed);
}

void motorBackward(int speed) {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  ledcWrite(0, speed);
  ledcWrite(1, speed);
}

void turnLeft(int speed) {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  ledcWrite(0, speed);
  ledcWrite(1, speed);
}

void turnRight(int speed) {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  ledcWrite(0, speed);
  ledcWrite(1, speed);
}

// === 初始化 ===
void setup() {
  Serial.begin(115200);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY,OUTPUT);

  digitalWrite(STBY,1);

  // PWM 通道設定
  ledcAttachPin(PWMA, 0);
  ledcAttachPin(PWMB, 1);
  ledcSetup(0, 1000, 8); // 1kHz, 8位元解析度
  ledcSetup(1, 1000, 8);

  leftEncoder.begin();
  rightEncoder.begin();
  leftEncoder.resetEncoder();
  rightEncoder.resetEncoder();

  Serial.println("🏁 TB6612FNG 自走車控制：w=前進 s=後退 a=左轉 d=右轉 r=停止");
  motorStop();
}

// === 主迴圈 ===
void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    switch (cmd) {
      case 'w':
        motorForward(180);
        Serial.println("🚗 前進");
        break;
      case 's':
        motorBackward(180);
        Serial.println("🔙 後退");
        break;
      case 'a':
        turnLeft(160);
        Serial.println("↩️ 左轉");
        break;
      case 'd':
        turnRight(160);
        Serial.println("↪️ 右轉");
        break;
      case 'r':
        motorStop();
        Serial.println("🛑 停止");
        break;
    }
  }

  // 顯示編碼器計數
  long leftCount = leftEncoder.get_count();
  long rightCount = rightEncoder.get_count();
  Serial.printf("左:%ld | 右:%ld\n", leftCount, rightCount);

  delay(200);
}
