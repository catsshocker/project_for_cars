#include "PIDController.h"

PIDController::PIDController(float kp, float ki, float kd) {
  setParams(kp, ki, kd);
}

void PIDController::setParams(float kp, float ki, float kd) {
  _kp = kp;
  _ki = ki;
  _kd = kd;
}

float PIDController::compute(float target, float actual) {
  float error = target - actual;
  _integral += error;
  float derivative = error - _prevError;
  _prevError = error;

  return _kp * error + _ki * _integral + _kd * derivative;
}

void PIDController::reset() {
  _prevError = 0;
  _integral = 0;
}
