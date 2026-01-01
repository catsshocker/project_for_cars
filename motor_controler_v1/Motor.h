// #ifndef MOTOR_H
// #define MOTOR_H

// #include <Arduino.h>
// #include <PIDController.h>

// enum controlMode { MODE_POWER,
//                    MODE_PID };

// class Motor {
// public:
//   Motor(int pwmPin, int dirPin1, int dirPin2, int encAPin, int encBPin);
//   void begin();
//   void setMode(ControlMode mode);
//   void setPID(float kp, float ki, float kd);

// private:
//   int _pwmPin, _dirPin1, _dirPin2;
//   int _encAPin, _encBPin;

//   volatile long _encoderCount = 0;
//   long _lastCount = 0;
//   unsigned long _lastTime = 0;

//   float _target = 0;
//   ControlMode _mode = MODE_PWM;
//   PIDController _pid;
// }

// #endif
