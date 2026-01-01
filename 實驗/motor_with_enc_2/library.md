# 專案說明文件

## 檔案結構

- EncoderPCNT.h / EncoderPCNT.cpp  
  編碼器計數器 PCNT 相關類別與函數
- PIDController.h / PIDController.cpp  
  PID 控制器類別與函數
- motor_with_enc_2.ino  
  主程式，馬達控制與編碼器整合
- library.md  
  說明文件

---

## EncoderPCNT 類別

**檔案**：[EncoderPCNT.h](EncoderPCNT.h), [EncoderPCNT.cpp](EncoderPCNT.cpp)

### 建構子

```cpp
EncoderPCNT(gpio_num_t pinA, gpio_num_t pinB, pcnt_unit_t unit);
```
- `pinA`：A 相腳位
- `pinB`：B 相腳位
- `unit`：PCNT 單元編號

### 成員函數

#### void begin()
初始化 PCNT 編碼器，設置 GPIO、事件、濾波器等。

#### int32_t get_count()
取得目前編碼器計數值（包含溢出累加）。

#### void resetEncoder()
重置編碼器計數值為 0。

### 重要成員變數

- `pcnt_unit_t _pcnt_unit`：PCNT 單元編號
- `int32_t _totalCount`：累積計數值

---

## PIDController 類別

**檔案**：[PIDController.h](PIDController.h), [PIDController.cpp](PIDController.cpp)

### 建構子

```cpp
PIDController(float kp = 1.0, float ki = 0.0, float kd = 0.0);
```
- `kp`：比例增益
- `ki`：積分增益
- `kd`：微分增益

### 成員函數

#### float compute(float target, float actual)
計算 PID 輸出  
- `target`：目標值  
- `actual`：實際值  
- 回傳：PID 控制輸出

#### void setParams(float kp, float ki, float kd)
設定 PID 參數

#### void reset()
重置積分與前次誤差

---

## motor 類別

**檔案**：[motor_with_enc_2.ino](motor_with_enc_2.ino)

### 建構子

```cpp
motor(gpio_num_t pinA, gpio_num_t pinB, gpio_num_t pin_pwm);
```
- `pinA`：馬達 A 相腳位
- `pinB`：馬達 B 相腳位
- `pin_pwm`：PWM 腳位

### 成員函數

#### void begin()
初始化馬達腳位

#### void setSpeed(int speed)
設定馬達速度  
- `speed`：速度值，範圍 -255 ~ 255，正負代表方向

#### void stop()
停止馬達  
- 依照 stop mode 決定 BRAKE 或 COAST

#### void setStopMode(motorstopmode mode)
設定停止模式  
- `mode`：BRAKE（煞車）或 COAST（滑行）

---

## 使用方法範例

```cpp
#include "EncoderPCNT.h"
#include "PIDController.h"

// 建立 EncoderPCNT 物件
EncoderPCNT encoder(GPIO_NUM_36, GPIO_NUM_39, PCNT_UNIT_0);
encoder.begin();
encoder.resetEncoder();
int enc = encoder.get_count();

// 建立 PIDController 物件
PIDController pid(1.0, 0.1, 0.01);
float output = pid.compute(target, actual);

// 建立 motor 物件
motor motor1(GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_14);
motor1.begin();
motor1.setStopMode(BRAKE);
motor1.setSpeed(255);
motor1.stop();
```

---

## 其他注意事項

- 所有 GPIO 腳位需依照硬體連接調整
- EncoderPCNT 需搭配 ESP32 PCNT 外設
- motor 類別 PWM 腳位需支援 analogWrite
- PIDController 適用於速度或位置閉迴路控制

如需更詳細範例，請參考 [motor_with_enc_2.ino](motor_with_enc_2.ino)。