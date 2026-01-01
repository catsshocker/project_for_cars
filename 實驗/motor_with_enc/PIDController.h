#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

class PIDController {
  public:
    PIDController(float kp = 1.0, float ki = 0.0, float kd = 0.0);

    float compute(float target, float actual);
    void setParams(float kp, float ki, float kd);
    void reset();

  private:
    float _kp, _ki, _kd;
    float _prevError = 0;
    float _integral = 0;
};

#endif
