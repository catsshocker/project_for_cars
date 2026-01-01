#ifndef HBRIDGE_H
#define HBRIDGE_H


#include "Arduino.h"
enum motorstopmode{
    BRAKE,
    COAST
};

class motor{
    public:
        motor();
        void setPin(int pinA, int pinB,int pin_pwm);
        void begin();
        void setSpeed(int speed);
        void stop();
        void setStopMode(motorstopmode mode);
        bool reverse = false;
    private:
        int _pinA = -1;
        int _pinB = -1;
        int _pin_pwm = -1;
        int _speed;
        motorstopmode _stopmode;
        bool _initialized = false;
};

class MotorController
{
private:
    bool _enabled;
    int _enablePin;
public:
    motor motorA;
    motor motorB;

    MotorController(int enablePin = -1);
    ~MotorController();
    void begin();
    void enable();
    void disable();
    bool isEnabled() { return _enabled; }

};

#endif
