#ifndef PIN_H
#define PIN_H

typedef enum{
    PINMODE_disable,
    PINMODE_INPUT,
    PINMODE_OUTPUT,
    PINMODE_INPUT_ultrasonic,
    PINMODE_ANALOG_input,
    PINMODE_encoder,
    PINMODE_PWM,
    PINMODE_uart,
    pinMODE_can,
} PinMode;

class Pin_Base
{
public:
    virtual void write(uint8_t value){};
    virtual uint8_t read(){ return 0;};
    virtual int read_ultlarsonic(){ return -1;};
};


class Pin : public Pin_Base
{
private:
    PinMode mode;
public:
    Pin : public Pin_Base();
    ~Pin : public Pin_Base();
    int pinMode(PinMode mode);
    
};

Pin : public Pin_Base::Pin : public Pin_Base(/* args */)
{
}

Pin : public Pin_Base::~Pin : public Pin_Base()
{
}



#endif