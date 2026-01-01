#include "EncoderPCNT.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===================================
// === WiFi & Server 設定 ===
// ===================================
// **請將此處替換為您的實際設定**
const char* ssid = "您的WiFi名稱";
const char* password = "您的WiFi密碼";
String serverUrl = "http://192.168.1.107:5000/update"; // 您的電腦 IP 和 Flask 伺服器埠號

// ===================================
// === 車輛狀態與控制變數 ===
// ===================================
int x = 0;       // 當前 X 座標 (已到達)
int y = 0;       // 當前 Y 座標 (已到達)
int target_x = 0; // 目標 X 座標 (伺服器給予)
int target_y = 0; // 目標 Y 座標 (伺服器給予)
int battery = 90; 
const char* car_id = "car1"; 

// === 狀態機與計時器 ===
bool isMoving = false; // 追蹤車輛是否正在執行移動任務
unsigned long previousMillis_http = 0;
const long interval_http = 1000; // 每 1 秒執行一次 HTTP POST 狀態回報/請求目標

// === 移動參數 (需要根據您的編碼器和車輪調整) ===
const int SPEED = 180;
// 假設移動一個座標單位 (一格) 需要 X 圈編碼器脈衝數
const long TICKS_PER_STEP = 500; 

// 暫存變數：用於記錄開始移動時的編碼器值
long start_left_ticks = 0;
long start_right_ticks = 0;

// ===================================
// === TB6612FNG 馬達腳位設定 ===
// [腳位定義與 EncoderPCNT 實例化保持不變]
// ===================================
#define STBY 26
#define AIN1 25
#define AIN2 33
#define PWMA 32
#define BIN1 27
#define BIN2 14
#define PWMB 12
EncoderPCNT leftEncoder(GPIO_NUM_35, GPIO_NUM_34, PCNT_UNIT_0);
EncoderPCNT rightEncoder(GPIO_NUM_39, GPIO_NUM_36, PCNT_UNIT_1);

// ===================================
// === 馬達控制函式 (保持不變) ===
// [motorStop, motorForward, motorBackward, turnLeft, turnRight 保持不變]
// ===================================
void motorStop() {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  ledcWrite(0, 0); ledcWrite(1, 0);
}
void motorForward(int speed) {
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
  ledcWrite(0, speed); ledcWrite(1, speed);
}
void motorBackward(int speed) {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
  ledcWrite(0, speed); ledcWrite(1, speed);
}
void turnLeft(int speed) {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
  ledcWrite(0, speed); ledcWrite(1, speed);
}
void turnRight(int speed) {
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
  ledcWrite(0, speed); ledcWrite(1, speed);
}


// ===================================
// === 核心：移動執行函式 ===
// ===================================

/**
 * @brief 根據當前位置和目標位置，決定馬達方向並驅動車輛，直到移動一格的距離。
 * @return bool 是否已到達目標距離 (完成一步移動)。
 */
bool executeMove() {
  // 1. 判斷是否還有距離需要移動
  if (x == target_x && y == target_y) {
    motorStop();
    isMoving = false;
    Serial.println("🛑 目標已到達。");
    return true; // 已經到達
  }

  // 2. 判斷移動方向 (這裡只處理單步移動，伺服器應只發送相鄰座標)
  int dx = target_x - x;
  int dy = target_y - y;

  // *** 注意：這裡只考慮移動一個座標單位的精準度 ***
  
  // 3. 檢查編碼器距離
  long current_left_ticks = abs(leftEncoder.get_count() - start_left_ticks);
  long current_right_ticks = abs(rightEncoder.get_count() - start_right_ticks);
  
  // 判斷是否達到預設步進距離
  if (current_left_ticks >= TICKS_PER_STEP && current_right_ticks >= TICKS_PER_STEP) {
      motorStop();
      return true; // 已完成一格移動
  }

  // 4. 啟動/維持馬達動作 (只有在剛啟動 isMoving=true 時，這裡才會被呼叫一次)
  if (dx == 1 && dy == 0) { // X 軸正向移動
    motorForward(SPEED);
  } else if (dx == -1 && dy == 0) { // X 軸負向移動
    motorBackward(SPEED);
  } else if (dy == 1 && dx == 0) { // Y 軸正向移動 (左轉)
    turnLeft(SPEED); 
  } else if (dy == -1 && dx == 0) { // Y 軸負向移動 (右轉)
    turnRight(SPEED);
  } else {
    // 伺服器發送了非相鄰的座標或無效指令，立即停止
    motorStop();
    Serial.printf("❌ 收到無效目標跳轉指令: (%d, %d) -> (%d, %d)\n", x, y, target_x, target_y);
    return true;
  }
  
  return false; // 尚未到達目標距離
}


// ===================================
// === HTTP POST 函式：上傳位置並接收新目標 ===
// ===================================
void updatePositionAndGetTarget() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi 未連線，無法上傳資料。");
    return;
  }
  
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  // 1. 建立上傳的 JSON 資料
  StaticJsonDocument<200> doc;
  doc["id"] = car_id;
  doc["x"] = x; // 上傳的是 '已到達' 的座標
  doc["y"] = y;
  doc["battery"] = battery;
  doc["status"] = isMoving ? "moving" : "idle"; // 回報當前狀態

  String jsonStr;
  serializeJson(doc, jsonStr);

  Serial.print("⬆️ 上傳位置: ");
  Serial.print(jsonStr);

  // 2. 執行 POST 請求
  int code = http.POST(jsonStr);

  if (code > 0) {
    String response = http.getString();
    Serial.println(" | ⬇️ 伺服器回傳: " + response);

    // 3. 解析伺服器回傳的下一步座標
    StaticJsonDocument<200> res;
    deserializeJson(res, response);

    int next_x = res["next_x"] | x; // 如果解析失敗，則保持當前座標
    int next_y = res["next_y"] | y;
    
    // **只有在車輛閒置時，才處理新的目標指令**
    if (!isMoving) {
        if (next_x != x || next_y != y) {
            target_x = next_x;
            target_y = next_y;
            isMoving = true; // 啟動移動任務
            
            // 記錄開始移動時的編碼器值，準備計數距離
            start_left_ticks = leftEncoder.get_count();
            start_right_ticks = rightEncoder.get_count();

            Serial.printf("🎯 收到新目標，啟動移動: (%d, %d)\n", target_x, target_y);
        } else {
            Serial.println("🎯 目標與當前位置相同，保持閒置。");
        }
    }
  } else {
    Serial.printf("❌ POST 請求失敗，錯誤碼: %d\n", code);
  }

  http.end();
}


// ===================================
// === 初始化 ===
// ===================================
void setup() {
  // [Setup 保持不變]
  Serial.begin(115200);

  // 馬達腳位設定
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT); digitalWrite(STBY, 1);

  // PWM 通道設定
  ledcAttachPin(PWMA, 0); ledcAttachPin(PWMB, 1);
  ledcSetup(0, 1000, 8); ledcSetup(1, 1000, 8); 

  // 編碼器設定
  leftEncoder.begin(); rightEncoder.begin();
  leftEncoder.resetEncoder(); rightEncoder.resetEncoder();

  // WiFi 連線 (省略連線細節)
  WiFi.begin(ssid, password);
  // ... (WiFi 連線迴圈) ...
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { delay(500); Serial.print("."); attempts++; }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi 連線成功!");
  } else {
    Serial.println("\n❌ WiFi 連線失敗，將在無網路模式下運行。");
  }

  Serial.println("\n🏁 自走車控制啟動");
  motorStop();
}

// ===================================
// === 主迴圈 ===
// ===================================
void loop() {
  // 1. 處理移動任務
  if (isMoving) {
    bool arrived = executeMove();

    if (arrived) {
      // 物理到達距離後，將當前座標更新為目標座標
      x = target_x; 
      y = target_y;
      isMoving = false; // 任務完成，進入閒置狀態
      motorStop(); 
      // 建議：在這裡立即執行一次 POST，回報已到達，而不是等待下一個計時器週期
      // updatePositionAndGetTarget(); 
      Serial.printf("✅ 成功到達座標: (%d, %d)，準備接收下一個指令。\n", x, y);
    }
  }

  // 2. 非阻塞式 HTTP 通訊 (定時回傳狀態或接收指令)
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis_http >= interval_http) {
    previousMillis_http = currentMillis;
    updatePositionAndGetTarget();
  }

  // 3. 顯示編碼器計數
  long leftCount = leftEncoder.get_count();
  long rightCount = rightEncoder.get_count();
  Serial.printf("狀態 | 移動中:%s | 左:%ld | 右:%ld | 位置:(%d,%d) | 目標:(%d,%d)\n",
                isMoving ? "是" : "否", leftCount, rightCount, x, y, target_x, target_y);
  delay(100); 
}