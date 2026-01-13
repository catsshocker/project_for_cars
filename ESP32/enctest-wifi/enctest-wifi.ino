#include "EncoderPCNT.h"
#include <Arduino.h>
#include "PIDController.h"
#include <WiFi.h>

// =====================
// WiFi & Server 設定
// =====================
const char* ssid = "莊家凱的iPhone";
const char* password = "Jason3240904";
const char* server_ip = "172.20.10.7";
const char* car_id = "car1";

// =====================
// 車輛狀態
// =====================
volatile int x = 0, y = 0;
volatile int target_x = 0, target_y = 0;
volatile bool isMoving = false;
volatile int SpeedA = 0, SpeedB = 0;
volatile int moveState = 0;  // 0=停止, 1=旋轉, 2=前進

unsigned long stateStartTime = 0;
long moveStartEncoderA = 0, moveStartEncoderB = 0;
#define TICKS_PER_UNIT 500   // 每一格距離對應的 Encoder 數

// =====================
// 腳位定義
// =====================
#define STBY 26
#define AIN1 25
#define AIN2 33
#define PWMA 32
#define BIN1 27
#define BIN2 14
#define PWMB 12

EncoderPCNT Encoder_A(GPIO_NUM_39, GPIO_NUM_36, PCNT_UNIT_0);
EncoderPCNT Encoder_B(GPIO_NUM_35, GPIO_NUM_34, PCNT_UNIT_1);

// =====================
// 馬達控制
// =====================
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

  analogWrite(PWMA, constrain(inSpeedA, 0, 255));
  analogWrite(PWMB, constrain(inSpeedB, 0, 255));
}

// =====================
// 馬達 PID 任務
// =====================
void motor_control_loop(void* parameter) {
  PIDController pidA(7, 0, 0.005);
  PIDController pidB(7, 0, 0.005);

  long lastTime = millis();
  double targetEncA = 0, targetEncB = 0;

  while(true) {
    long now = millis();
    double dt = (now - lastTime) / 1000.0;
    lastTime = now;
    if(dt <= 0) dt = 0.02;

    if(SpeedA == 0 && SpeedB == 0) {
      targetEncA = Encoder_A.get_count();
      targetEncB = Encoder_B.get_count();
    }

    targetEncA += SpeedA * dt * 6.0;
    targetEncB += SpeedB * dt * 6.0;

    int controlA = pidA.compute(targetEncA, Encoder_A.get_count(), 255, -255, dt);
    int controlB = pidB.compute(targetEncB, Encoder_B.get_count(), 255, -255, dt);

    setmotorSpeed(controlA, controlB);
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

// =====================
// 通訊 & 狀態任務
// =====================
void speed_listen_task(void* parameter) {
  while(true) {
    // 1️⃣ 從伺服器取得下一步目標
    if(WiFi.status() == WL_CONNECTED && !isMoving) {
      WiFiClient client;
      if(client.connect(server_ip, 5000)) {
        String url = "/update?id=" + String(car_id) +
                     "&x=" + String(x) +
                     "&y=" + String(y);
        client.print(String("GET ") + url +
                     " HTTP/1.1\r\nHost: " + server_ip +
                     "\r\nConnection: close\r\n\r\n");

        unsigned long timeout = millis();
        while(!client.available() && millis() - timeout < 2000);

        String response = "";
        while(client.available()) {
          response += (char)client.read();
        }
        client.stop();

        response.trim();  // 去掉前後空白與換行

        int comma1 = response.indexOf(',');
        int comma2 = response.indexOf(',', comma1 + 1);

        if(comma1 > 0 && comma2 > comma1) {
          int nextX = response.substring(0, comma1).toInt();
          int nextY = response.substring(comma1 + 1, comma2).toInt();

          Serial.printf("[PARSED DATA] %d,%d\n", nextX, nextY);

          if(nextX != x || nextY != y) {
            target_x = nextX;
            target_y = nextY;
            isMoving = true;
            moveState = 1;
            stateStartTime = millis();
            Serial.printf("[NEW TARGET] (%d, %d)\n", target_x, target_y);
          }
        } else {
          Serial.printf("[PARSE ERROR] format error: '%s'\n", response.c_str());
        }
      } else {
        Serial.println("[ERROR] Server offline");
      }
    }

    // 2️⃣ 執行移動狀態機
    if(isMoving) {
      if(moveState == 1) {  // 旋轉
        SpeedA = 80;
        SpeedB = 80;
        if(millis() - stateStartTime > 800) {
          moveState = 2;
          moveStartEncoderA = Encoder_A.get_count();
          moveStartEncoderB = Encoder_B.get_count();
          Serial.println("[MOVE] Rotate Done -> Forward");
        }
      } else if(moveState == 2) {  // 前進
        SpeedA = 150;
        SpeedB = -150;

        long deltaA = abs(Encoder_A.get_count() - moveStartEncoderA);
        long deltaB = abs(Encoder_B.get_count() - moveStartEncoderB);

        if(deltaA >= TICKS_PER_UNIT || deltaB >= TICKS_PER_UNIT) {
          SpeedA = 0;
          SpeedB = 0;
          moveState = 0;
          isMoving = false;
          x = target_x;
          y = target_y;
          Serial.printf("[ARRIVED] (%d, %d)\n", x, y);
        }
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// =====================
// 初始化
// =====================
void setup() {
  Serial.begin(115200);

  pinMode(STBY, OUTPUT); digitalWrite(STBY, 1);
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);

  Encoder_A.begin();
  Encoder_B.begin();

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  xTaskCreatePinnedToCore(motor_control_loop, "motor", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(speed_listen_task, "listen", 4096, NULL, 1, NULL, 1);
}

void loop() {}
