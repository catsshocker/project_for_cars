#include "EncoderPCNT.h"

EncoderPCNT encoder(GPIO_NUM_36, GPIO_NUM_39, PCNT_UNIT_0);

enum motorstopmode{
  BRAKE,
  COAST
};

class motor{
  public:
    motor(gpio_num_t pinA, gpio_num_t pinB,gpio_num_t pin_pwm);
    void begin();
    void setSpeed(int speed);
    // int getSpeed();
    void stop();
    void setStopMode(motorstopmode mode);
  private:
    gpio_num_t _pinA;
    gpio_num_t _pinB;
    gpio_num_t _pin_pwm;
    int _speed;
    motorstopmode _stopmode;
};

motor motor1(GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_14);

void setup() {
  encoder.begin();
  Serial.begin(115200);
  pinMode(12, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(12, HIGH); 
  motor1.begin();
  

  // int i = 0;


  encoder.resetEncoder();
  motor1.setStopMode(BRAKE);
  motor1.setSpeed(255);
  delay(1000);

  motor1.stop();
  delay(200);
  int enc = encoder.get_count();
  Serial.println(enc);
  

  digitalWrite(2, HIGH);
  
}

void loop() {
  // put your main code here, to run repeatedly:

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
  if(speed > 255){
    speed = 255;
  }else if(speed < -255){
    speed = -255;
  }
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
