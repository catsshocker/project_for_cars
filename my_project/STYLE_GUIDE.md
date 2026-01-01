# 📘 Arduino 專案語法風格指南

## 📁 一、檔案命名規範

- 使用小寫字母與底線分隔
- 每個模組使用一組 `.h` + `.cpp`
- `.ino` 主程式檔以功能命名，避免 `sketch_...` 類型命名

| 類型       | 範例                |
|------------|---------------------|
| 標頭檔     | `motor_control.h`   |
| 實作檔     | `motor_control.cpp` |
| 主程式檔   | `robot_arm.ino`     |

## 🔤 二、命名規則

### 📦 類別名稱：大駝峰式（PascalCase）
```cpp
class MotorDriver {};
class SensorReader {};
```

### 🔧 函式名稱：小駝峰式（camelCase）
```cpp
void setSpeed(int speed);
int readSensorData();
```

### 🧩 變數名稱：小駝峰式 + 私有變數加底線
```cpp
int speedValue;
int _pwmPin;  // 類別內部變數
```

### 🚨 常數 / 巨集名稱：全大寫 + 底線分隔
```cpp
#define MAX_SPEED 255
const int DEFAULT_TIMEOUT = 1000;
```

## 🔧 三、程式格式與排版

- 使用 Allman Style 括號（大括號換行）
- 縮排建議使用 4 個空格（或 tab）
- 空行分隔邏輯區塊

```cpp
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);

    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
```

## 🗂 四、專案架構建議

```
your_project/
├── src/
│   ├── motor_control.cpp
│   └── motor_control.h
├── include/
├── main.ino
├── README.md
├── STYLE_GUIDE.md
└── .gitignore
```

## ✍️ 五、註解規範

### 單行註解
```cpp
// 初始化 LED 腳位
pinMode(LED_BUILTIN, OUTPUT);
```

### 區塊註解
```cpp
/*
 * 控制馬達轉速
 * - speed > 0 時正轉
 * - speed < 0 時反轉
 * - speed = 0 時停止
 */
void setSpeed(int speed)
{
    ...
}
```

## 🧠 六、撰寫順序建議

1. `setup()` 與 `loop()` 最上面
2. 常用功能函式在中間
3. 輔助函式、類別實作移至 `.cpp` 檔中

## ✅ 七、版本控制習慣

- 一個儲存庫包含多模組，集中管理（例如 `esp32-libs`）
- 每個模組分為 `.h/.cpp`
- 建議配合 Git commit message 格式，例如：
  ```
  feat: 新增 MotorControl 類別
  fix: 修正 I2C 設定錯誤
  refactor: 重構速度控制邏輯
  ```