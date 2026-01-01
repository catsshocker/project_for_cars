#include <Arduino.h>
#include "Hbridge.h"

// 🌀 ESP32 的 PWM 通道與設定
#define MOTOR_A_PWM_CH 0
#define MOTOR_B_PWM_CH 1
#define PWM_FREQ 1000     // 1kHz 頻率
#define PWM_RESOLUTION 8  // 8位元 (0~255)

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
    if (_initialized) {
        pinMode(_pinA, OUTPUT);
        pinMode(_pinB, OUTPUT);
        pinMode(_pin_pwm, OUTPUT);
        digitalWrite(_pinA, LOW);
        digitalWrite(_pinB, LOW);

        // ⚙️ ESP32 PWM 初始化（分配通道）
        static uint8_t nextChannel = 0;
        _pwmChannel = nextChannel++;
        if (_pwmChannel > 15) _pwmChannel = 15; // 最多 16 通道

        ledcSetup(_pwmChannel, PWM_FREQ, PWM_RESOLUTION);
        ledcAttachPin(_pin_pwm, _pwmChannel);
    }
}

void motor::setSpeed(int speed)
{
    if (_initialized) {
        if (speed > 255) speed = 255;
        if (speed < -255) speed = -255;
        if (reverse) speed = -speed;

        if (speed > 0) {
            digitalWrite(_pinA, HIGH);
            digitalWrite(_pinB, LOW);
        }
        else if (speed < 0) {
            digitalWrite(_pinA, LOW);
            digitalWrite(_pinB, HIGH);
        }
        else {
            stop();
        }

        // ⚡ 使用 ESP32 專用 PWM 輸出
        ledcWrite(_pwmChannel, abs(speed));
        _speed = speed;
    }
}

void motor::stop()
{
    if (_initialized) {
        if (_stopmode == BRAKE) {
            digitalWrite(_pinA, HIGH);
            digitalWrite(_pinB, HIGH);
        } else {
            digitalWrite(_pinA, LOW);
            digitalWrite(_pinB, LOW);
        }
        ledcWrite(_pwmChannel, 0);
        _speed = 0;
    }
}

void motor::setStopMode(motorstopmode mode)
{
    _stopmode = mode;
}


// ---------------- MotorController ----------------
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
