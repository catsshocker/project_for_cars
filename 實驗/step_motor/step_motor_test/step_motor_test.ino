#include <AccelStepper.h>

// 定義 TB6600 接法
#define STEP_PIN 14   // PUL−
#define DIR_PIN  13   // DIR−
#define ENABLE_PIN 12 // ENA−

int a= 1;

// 建立 AccelStepper 物件 (DRIVER 模式: STEP+DIR)
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

bool motorRunning = false; // 追蹤馬達狀態

void setup() {
  Serial.begin(115200);

  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // 低電位啟用 TB6600

  stepper.setMaxSpeed(1000); // 最大速度 (步/秒)
  stepper.setAcceleration(500); // 加速度 (步/秒^2)

  Serial.println("Stepper ready. Type 'start' to run, 'stop' to halt.");
}

void loop() {
  // 檢查 Serial 指令
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim(); // 去掉空白
    if (cmd == "start") {
      motorRunning = true;
      a=1;
      Serial.println("Motor started.");
    } else if (cmd == "stop") {
      motorRunning = false;
      Serial.println("Motor stopped.");
    } else if (cmd == "start0") {
      a=-1;
      motorRunning = true;
      Serial.println("Motor tb.");
    } else {
      Serial.println("Unknown command. Use 'start' or 'stop'.");
    }
  }

  // 控制馬達轉動
  if (motorRunning) {
    stepper.setSpeed(500*a); // 設定速度 (正數為順時針)
    stepper.runSpeed();     // 持續以設定速度轉動
  }
}
