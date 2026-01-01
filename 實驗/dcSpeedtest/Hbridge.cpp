#include "Hbridge.h"

motor::motor() : _pinA(-1), _pinB(-1), _pin_pwm(-1), _speed(0), _stopmode(COAST), _initialized(false) {}

void motor::setPin(int pinA, int pinB, int pin_pwm)
{
    _pinA = pinA;
    _pinB = pinB;
    _pin_pwm = pin_pwm;
    _speed = 0;
    _stopmode = COAST;
    _initialized = true;
}

void motor::begin()
{
    if(_initialized){
        pinMode(_pinA, OUTPUT);
        pinMode(_pinB, OUTPUT);
        pinMode(_pin_pwm, OUTPUT);
        digitalWrite(_pinA, LOW);
        digitalWrite(_pinB, LOW);
    }
}

void motor::setSpeed(int speed)
{
    if(_initialized){
        if (speed > 255) speed = 255;
        if (speed < -255) speed = -255;
        if(reverse) speed = -speed;
        if (speed > 0)
        {
            digitalWrite(_pinA, HIGH);
            digitalWrite(_pinB, LOW);
        }
        else if (speed < 0)
        {
            digitalWrite(_pinA, LOW);
            digitalWrite(_pinB, HIGH);
        }
        else
        {
            stop();
        }
        analogWrite(_pin_pwm, abs(speed));
        _speed = speed;
    }
}

void motor::stop()
{
    if(_initialized){
            if (_stopmode == BRAKE)
        {
            digitalWrite(_pinA, HIGH);
            digitalWrite(_pinB, HIGH);
        }
        else
        {
            digitalWrite(_pinA, LOW);
            digitalWrite(_pinB, LOW);
        }
        _speed = 0;
    }
}

void motor::setStopMode(motorstopmode mode)
{
    _stopmode = mode;
}



MotorController::MotorController(int enablePin)
{
    _enabled = false;
    _enablePin = enablePin;
    if (_enablePin != -1)
    {
        pinMode(_enablePin, OUTPUT);
    }
}

MotorController::~MotorController()
{
    disable();
}

void MotorController::begin()
{
    motorA.begin();
    motorB.begin();
    disable();
}

void MotorController::enable()
{
    if (_enablePin != -1)
    {
        digitalWrite(_enablePin, HIGH);
    }
    _enabled = true;
}

void MotorController::disable()
{
    if (_enablePin != -1)
    {
        digitalWrite(_enablePin, LOW);
    }
    motorA.stop();
    motorB.stop();
    _enabled = false;
}