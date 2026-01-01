#include "EncoderPCNT.h"

EncoderPCNT encoder(GPIO_NUM_39, GPIO_NUM_36, PCNT_UNIT_0);

enum motorstopmode{
  BRAKE,
  COAST
};

class motor{
  public:
    motor(gpio_num_t pinA, gpio_num_t pinB,gpio_num_t pin_pwm);
    void begin();
    void setSpeed(int speed);
    void stop();
    void setStopMode(motorstopmode mode);
  private:
    gpio_num_t _pinA;
    gpio_num_t _pinB;
    gpio_num_t _pin_pwm;
    int _speed;
    motorstopmode _stopmode;
};

motor motor1(GPIO_NUM_25, GPIO_NUM_33, GPIO_NUM_32);

int LED = 2;
int STBY = 26;

// ===== 新增：目標位置控制變數 =====
long target = 1015; // 目標 encoder 數（例如約一圈）
float Kp = 0.3;     // 比例常數（調整靈敏度，建議 0.1~0.5）
int minPWM = 70;    // 最小啟動 PWM（馬達靜摩擦要克服）
int tolerance = 10; // 誤差範圍，±10 counts 內算達標

//1320 no
//1144 no

void setup() {
  encoder.begin();
  Serial.begin(115200);
  pinMode(STBY, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  digitalWrite(STBY, HIGH); 
  motor1.begin();

  encoder.resetEncoder();
  //otor1.setStopMode(BRAKE);
  motor1.setStopMode(COAST);

  Serial.println("開始轉到目標位置...");
}

void loop() {
  long pos = encoder.get_count();
  long error = target - pos;

  // 簡單 P 控制
  float control = Kp * error;

  // 限制範圍 -255~255
  if (control > 150) control = 150;
  if (control < -150) control = -150;

  // 加入最小轉動閾值（避免太小不動）
  if (abs(control) < minPWM && abs(error) > tolerance) {
    control = (control > 0) ? minPWM : -minPWM;
  }

  // 若已達目標附近就停止
  if (abs(error) <= tolerance) {
    motor1.stop();
    Serial.printf("到達目標！目前位置: %ld 誤差: %ld\n", pos, error);
    delay(1000);
    return; // 停止 loop（或可改成讓他重新動作）
  }

  motor1.setSpeed((int)control);

  // 印出 debug
  Serial.printf("pos=%ld, error=%ld, pwm=%.1f\n", pos, error, control);
  delay(20); // 更新頻率約 50Hz
}

motor::motor(gpio_num_t pinA, gpio_num_t pinB, gpio_num_t pin_pwm){
  _pinA = pinA;
  _pinB = pinB;
  _pin_pwm = pin_pwm;
  _speed = 0;
  _stopmode = COAST;
}

void motor::begin(){
  pinMode(_pinA, OUTPUT);
  pinMode(_pinB, OUTPUT);
  pinMode(_pin_pwm, OUTPUT);
  digitalWrite(_pinA, LOW);
  digitalWrite(_pinB, LOW);
}

void motor::setSpeed(int speed){
  if(speed > 0){
    digitalWrite(_pinA, HIGH);
    digitalWrite(_pinB, LOW);
  }else if(speed < 0){
    digitalWrite(_pinA, LOW);
    digitalWrite(_pinB, HIGH);
  }else{
    stop();
  }
  if(speed > 255) speed = 255;
  if(speed < -255) speed = -255;
  analogWrite(_pin_pwm, abs(speed));
  _speed = speed;
}

void motor::stop(){
  if(_stopmode == BRAKE){
    digitalWrite(_pinA, HIGH);
    digitalWrite(_pinB, HIGH);
  }else{
    digitalWrite(_pinA, LOW);
    digitalWrite(_pinB, LOW);
  }
  _speed = 0;
}

void motor::setStopMode(motorstopmode mode){
  _stopmode = mode;
}
