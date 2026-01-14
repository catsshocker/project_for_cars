#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "EncoderPCNT.h"
#include "PIDController.h"

// --- WiFi 設定 ---
const char* ssid = "Jason_iPhone";
const char* password = "Jason3240904";
const String server_ip = "172.20.10.7";

// --- 腳位與參數 ---
#define STBY 26
#define AIN1 25
#define AIN2 33
#define PWMA 32
#define BIN1 27
#define BIN2 14
#define PWMB 12

// 你的物理比例參數
#define FRONT_RADIO 8.5
#define ANGLE_RADIO (248.0/360.0)

// 全域變數
int SpeedA = 0;
int SpeedB = 0;
float curX = 0, curY = 0, tarX_g = 0, tarY_g = 0;
bool needToReport = false;

EncoderPCNT Encoder_A(GPIO_NUM_39, GPIO_NUM_36, PCNT_UNIT_0);
EncoderPCNT Encoder_B(GPIO_NUM_35, GPIO_NUM_34, PCNT_UNIT_1);
WebServer server(80);

// --- 你的梯形規劃類別 (MotionState, TrapezoidalProfile) 保持不變 ---
struct MotionState { double pos; double vel; };
class TrapezoidalProfile {
    // ... (這裡請貼上你原本的 TrapezoidalProfile 完整內容) ...
};

// --- 馬達底層控制 ---
void setmotorSpeed(int inSpeedA, int inSpeedB) {
    // 這裡加入 minPWM 防死區邏輯
    int minPWM = 80; 
    if (inSpeedA != 0 && abs(inSpeedA) < minPWM) inSpeedA = (inSpeedA > 0) ? minPWM : -minPWM;
    if (inSpeedB != 0 && abs(inSpeedB) < minPWM) inSpeedB = (inSpeedB > 0) ? minPWM : -minPWM;

    digitalWrite(AIN1, inSpeedA >= 0); digitalWrite(AIN2, inSpeedA < 0);
    digitalWrite(BIN1, inSpeedB >= 0); digitalWrite(BIN2, inSpeedB < 0);
    analogWrite(PWMA, constrain(abs(inSpeedA), 0, 255));
    analogWrite(PWMB, constrain(abs(inSpeedB), 0, 255));
}

// --- 導航回報機制 ---
void askNext() {
    HTTPClient http;
    http.setTimeout(5000);
    String url = "http://" + server_ip + ":5000/update?id=car1&x=" + String(curX) + "&y=" + String(curY);
    http.begin(url);
    if (http.GET() > 0) Serial.println(">>> 導航回報成功");
    http.end();
}

// --- 結合梯形規劃的移動邏輯 ---
void handleMove() {
    tarX_g = server.arg("x").toFloat();
    tarY_g = server.arg("y").toFloat();
    
    float dx = (tarX_g - curX) * 20.0; // 假設一格 20 cm
    float dy = (tarY_g - curY) * 20.0;
    float d = sqrt(dx*dx + dy*dy);
    float ang = atan2(dy, dx) * 180.0 / PI;

    // 先回覆 Flask，然後開始執行移動（避免 Flask 等太久逾時）
    server.send(200, "text/plain", "Moving");

    // 1. 執行旋轉
    TrapezoidalProfile t_prof(abs(ang) * ANGLE_RADIO, 180, 150, 0.02);
    while (!t_prof.isFinished()) {
        MotionState s = t_prof.update();
        SpeedA = (int)s.vel * (ang > 0 ? 1 : -1);
        SpeedB = (int)s.vel * (ang > 0 ? 1 : -1); // 旋轉時同向（原地自旋）
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // 2. 執行前進
    TrapezoidalProfile f_prof(d * FRONT_RADIO, 200, 150, 0.02);
    while (!f_prof.isFinished()) {
        MotionState s = f_prof.update();
        SpeedA = (int)s.vel;
        SpeedB = -(int)s.vel; // 前進時反向
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    SpeedA = 0; SpeedB = 0;
    curX = tarX_g; curY = tarY_g;
    needToReport = true; // 標記需要回報
}

// --- PID 任務 (Core 1) ---
void motor_control_loop(void* p) {
    PIDController pidA(7, 0, 0.005);
    PIDController pidB(7, 0, 0.005);
    long lastTime = millis();
    int targetA = Encoder_A.get_count();
    int targetB = Encoder_B.get_count();

    while (true) {
        long now = millis();
        double dt = (now - lastTime) / 1000.0;
        lastTime = now;

        targetA += SpeedA * dt * 6;
        targetB += SpeedB * dt * 6;

        int ctrlA = pidA.compute(targetA, Encoder_A.get_count(), 255, -255, dt);
        int ctrlB = pidB.compute(targetB, Encoder_B.get_count(), 255, -255, dt);
        
        setmotorSpeed(ctrlA, ctrlB);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(STBY, OUTPUT); digitalWrite(STBY, 1);
    pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
    
    Encoder_A.begin(); Encoder_B.begin();
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nWiFi Connected. IP: " + WiFi.localIP().toString());

    server.on("/move", handleMove);
    server.begin();

    xTaskCreatePinnedToCore(motor_control_loop, "motor", 4096, NULL, 1, NULL, 1);
}

void loop() {
    server.handleClient();
    if (needToReport) {
        askNext();
        needToReport = false;
    }
    yield();
}