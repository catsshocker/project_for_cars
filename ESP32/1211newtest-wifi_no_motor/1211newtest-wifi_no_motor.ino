#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ==========================
// 無馬達實體測試版本
// - 基於 1211newtest-wifi.ino
// - 移除馬達與編碼器依賴，改為模擬移動
// - 用於測試與伺服器互通、路徑回傳行為
// ==========================

// ===== WiFi & Server 設定 =====
const char* ssid = "cinosba";
const char* password = "063026366";
String serverUrl = "http://192.168.1.107:5000/update"; // 改成你的伺服器 IP

// ===== 車輛狀態 =====
int x = 0;       // 當前 X 座標 (已到達)
int y = 0;       // 當前 Y 座標 (已到達)
int target_x = 0; // 伺服器給的目標 X
int target_y = 0; // 伺服器給的目標 Y
int battery = 90;
const char* car_id = "car1";

// 狀態機與定時器
bool isMoving = false;
unsigned long previousMillis_http = 0;
const long interval_http = 1000; // 每 1 秒請求一次

// 模擬移動延遲（毫秒），代表從收到 next -> 到達所需時間
const unsigned long MOVE_SIM_DELAY = 800;
unsigned long moveStartMillis = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n===== 無馬達測試啟動 =====");
  Serial.printf("SSID: %s\n", ssid);

  // 連 WiFi
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print('.');
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi 連線成功");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi 連線失敗");
  }

  Serial.println("啟動完成，等待伺服器指令...");
}

// 模擬物理移動：在 MOVE_SIM_DELAY 後視為抵達目標
void simulateMovement() {
  if (!isMoving) return;
  unsigned long now = millis();
  if (now - moveStartMillis >= MOVE_SIM_DELAY) {
    // 到達
    x = target_x;
    y = target_y;
    isMoving = false;
    Serial.printf("✅ 模擬到達座標 (%d, %d)\n", x, y);
    // 立即回報一次到達（可選）
    updatePositionAndGetTarget();
  }
}

void updatePositionAndGetTarget() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi 未連線，無法上傳資料。");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["id"] = car_id;
  doc["x"] = x;
  doc["y"] = y;
  doc["battery"] = battery;
  doc["status"] = isMoving ? "moving" : "idle";

  String payload;
  serializeJson(doc, payload);

  Serial.print("⬆️ 上傳: "); Serial.println(payload);

  int code = http.POST(payload);
  if (code > 0) {
    String res = http.getString();
    Serial.print("⬇️ 回應: "); Serial.println(res);

    // 解析
    StaticJsonDocument<200> resp;
    DeserializationError err = deserializeJson(resp, res);
    if (err) {
      Serial.print("⚠️ 解析回應失敗: "); Serial.println(err.c_str());
      http.end();
      return;
    }

    int next_x = resp["next_x"] | x;
    int next_y = resp["next_y"] | y;

    // 只有在閒置時才接受新移動任務
    if (!isMoving) {
      if (next_x != x || next_y != y) {
        target_x = next_x;
        target_y = next_y;
        isMoving = true;
        moveStartMillis = millis();
        Serial.printf("🎯 新目標: (%d, %d)，開始模擬移動\n", target_x, target_y);
      } else {
        Serial.println("🎯 無新目標，保持閒置");
      }
    } else {
      Serial.println("🔁 目前正在移動，忽略新指令");
    }

  } else {
    Serial.printf("❌ POST 失敗，錯誤碼: %d\n", code);
  }
  http.end();
}

void loop() {
  // 每秒請求一次（非阻塞）
  unsigned long now = millis();
  if (now - previousMillis_http >= interval_http) {
    previousMillis_http = now;
    updatePositionAndGetTarget();
  }

  // 模擬移動進度
  simulateMovement();

  // 印出狀態
  Serial.printf("狀態 | 移動中:%s | 位置:(%d,%d) | 目標:(%d,%d)\n",
                isMoving ? "是" : "否", x, y, target_x, target_y);
  delay(100);
}
