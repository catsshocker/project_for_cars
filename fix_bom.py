#!/usr/bin/env python3
# -*- coding: utf-8 -*-

code = '''#include "EncoderPCNT.h"
#include <Arduino.h>
#include "PIDController.h"
#include <WiFi.h>

const char* ssid = "莊家凱的iPhone";
const char* password = "Jason3240904";
const char* car_id = "car1";

int x = 0, y = 0, target_x = 0, target_y = 0;
bool isMoving = false;
int SpeedA = 0, SpeedB = 0;

int moveState = 0;
unsigned long stateStartTime = 0;
long moveStartEncoderA = 0, moveStartEncoderB = 0;
#define TICKS_PER_UNIT 500

#define STBY 26
#define AIN1 25
#define AIN2 33
#define PWMA 32
#define BIN1 27
#define BIN2 14
#define PWMB 12

EncoderPCNT Encoder_A(GPIO_NUM_39, GPIO_NUM_36, PCNT_UNIT_0);
EncoderPCNT Encoder_B(GPIO_NUM_35, GPIO_NUM_34, PCNT_UNIT_1);

void motorStop() {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  analogWrite(PWMA, 0); analogWrite(PWMB, 0);
}

void setmotorSpeed(int inSpeedA, int inSpeedB) {
  if(inSpeedA < 0) {
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    inSpeedA = -inSpeedA;
  } else {
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
  }
  if(inSpeedB < 0) {
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    inSpeedB = -inSpeedB;
  } else {
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
  }
  if(inSpeedA > 255) inSpeedA = 255;
  if(inSpeedB > 255) inSpeedB = 255;

  analogWrite(PWMA, inSpeedA);
  analogWrite(PWMB, inSpeedB);
}

void executeMovement() {
  if(moveState == 0) {
    SpeedA = 0;
    SpeedB = 0;
    return;
  }
  
  if(moveState == 1) {
    SpeedA = 80;
    SpeedB = 80;
    unsigned long elapsed = millis() - stateStartTime;
    
    if(elapsed > 800) {
      moveState = 2;
      moveStartEncoderA = Encoder_A.get_count();
      moveStartEncoderB = Encoder_B.get_count();
      Serial.printf("[ROTATE] Complete at %lums | Start Encoders A:%ld B:%ld\n", 
                    elapsed, moveStartEncoderA, moveStartEncoderB);
    }
  }
  else if(moveState == 2) {
    SpeedA = 150;
    SpeedB = -150;
    
    long currentA = Encoder_A.get_count();
    long currentB = Encoder_B.get_count();
    long deltaA = abs(currentA - moveStartEncoderA);
    long deltaB = abs(currentB - moveStartEncoderB);
    
    static unsigned long lastLog = 0;
    if(millis() - lastLog > 1000) {
      Serial.printf("[FORWARD] Progress: A=%ld/%d  B=%ld/%d  (Raw: A=%ld  B=%ld)\n", 
                    deltaA, TICKS_PER_UNIT, deltaB, TICKS_PER_UNIT, currentA, currentB);
      lastLog = millis();
    }
    
    if(deltaA >= TICKS_PER_UNIT || deltaB >= TICKS_PER_UNIT) {
      SpeedA = 0;
      SpeedB = 0;
      moveState = 0;
      x = target_x;
      y = target_y;
      isMoving = false;
      Serial.printf("[ARRIVED] Reached (%d, %d) | Final Delta: A=%ld  B=%ld\n", 
                    x, y, deltaA, deltaB);
    }
  }
}

void updatePositionAndGetTarget() {
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("[WIFI] Not connected - skipping request");
    return;
  }
  
  if(isMoving) {
    static unsigned long lastStatus = 0;
    if(millis() - lastStatus > 2000) {
      Serial.printf("[STATUS] Moving state=%d | Target:(%d,%d) Current:(%d,%d)\n",
                    moveState, target_x, target_y, x, y);
      lastStatus = millis();
    }
    return;
  }
  
  Serial.println("[HTTP] Requesting target from server...");
  
  WiFiClient client;
  if(!client.connect("172.20.10.7", 5000)) {
    Serial.println("[HTTP] Connection failed");
    return;
  }
  
  String url = "/update?id=" + String(car_id) + "&x=" + String(x) + "&y=" + String(y);
  Serial.printf("[HTTP] GET /update?id=%s&x=%d&y=%d\n", car_id, x, y);
  
  client.print(String("GET ") + url + " HTTP/1.1\r\nHost: 172.20.10.7\r\nConnection: close\r\n\r\n");
  
  String response = "";
  while(client.available()) response += char(client.read());
  client.stop();
  
  int lastNewline = response.lastIndexOf('\n');
  if(lastNewline > 0) {
    String data = response.substring(lastNewline + 1);
    data.trim();
    
    Serial.printf("[HTTP] Response body: %s\n", data.c_str());
    
    int comma1 = data.indexOf(',');
    int comma2 = data.indexOf(',', comma1 + 1);
    
    if(comma1 > 0 && comma2 > 0) {
      int next_x = data.substring(0, comma1).toInt();
      int next_y = data.substring(comma1 + 1, comma2).toInt();
      
      Serial.printf("[PARSE] OK - next_x=%d, next_y=%d\n", next_x, next_y);
      
      if(next_x != x || next_y != y) {
        target_x = next_x;
        target_y = next_y;
        isMoving = true;
        moveState = 1;
        stateStartTime = millis();
        Serial.printf("[MOVE] Started movement from (%d,%d) to (%d,%d)\n", x, y, target_x, target_y);
      } else {
        Serial.println("[PARSE] Target same as current - no movement");
      }
    } else {
      Serial.printf("[PARSE] FAILED - comma1=%d, comma2=%d\n", comma1, comma2);
    }
  } else {
    Serial.println("[HTTP] No response body received");
  }
}

void motor_control_loop(void* parameter) {
  PIDController pidA(7, 0, 0.005);
  PIDController pidB(7, 0, 0.005);
  long lastTime = millis();
  int targetEncoderA = 0, targetEncoderB = 0;
  unsigned long loopCounter = 0;
  
  Serial.println("[MOTOR] Control loop started");
  
  while(true) {
    long currentTime = millis();
    double dt = (currentTime - lastTime) / 1000.0;
    lastTime = currentTime;
    
    int currentEncoderA = Encoder_A.get_count();
    int currentEncoderB = Encoder_B.get_count();
    
    targetEncoderA += SpeedA * dt * 6;
    targetEncoderB += SpeedB * dt * 6;
    
    int controlA = pidA.compute(targetEncoderA, currentEncoderA, 255, -255, dt);
    int controlB = pidB.compute(targetEncoderB, currentEncoderB, 255, -255, dt);
    
    setmotorSpeed(controlA, controlB);
    
    if(++loopCounter % 100 == 0) {
      Serial.printf("[MOTOR] Speed: A=%d B=%d | PWM: A=%d B=%d | Enc: A=%ld B=%ld\n",
                    SpeedA, SpeedB, controlA, controlB, currentEncoderA, currentEncoderB);
    }
    
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void speed_listen_task(void* parameter) {
  while(true) {
    updatePositionAndGetTarget();
    executeMovement();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  
  Serial.println("\n===============================================");
  Serial.println("   ESP32 Auto-Vehicle Control System");
  Serial.println("===============================================");
  Serial.printf("Car ID: %s\n", car_id);
  Serial.printf("WiFi: %s\n", ssid);
  Serial.println("===============================================\n");
  
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT); digitalWrite(STBY, 1);
  Serial.println("[INIT] Motor pins configured");

  Encoder_A.begin();
  Encoder_B.begin();
  Encoder_A.resetEncoder();
  Encoder_B.resetEncoder();
  Serial.println("[INIT] Encoders initialized");

  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting");
  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println(" OK");
    Serial.printf("[WIFI] IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println(" FAILED");
  }

  Serial.println("[TASK] Creating motor control loop...");
  xTaskCreatePinnedToCore(motor_control_loop, "motor", 4096, NULL, 1, NULL, 1);
  
  Serial.println("[TASK] Creating speed listen task...");
  xTaskCreatePinnedToCore(speed_listen_task, "listen", 4096, NULL, 1, NULL, 1);
  
  Serial.println("\n===============================================");
  Serial.println("   System Ready! Waiting for commands...");
  Serial.println("===============================================\n");
}

void loop() {}
'''

# 寫入文件，使用 UTF-8 不加 BOM
with open(r"d:\專題\ESP32\enctest-wifi\enctest-wifi.ino", "w", encoding="utf-8") as f:
    f.write(code)

print("✓ File written successfully without BOM")
