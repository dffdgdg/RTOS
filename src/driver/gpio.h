#pragma once
#include <Arduino.h>

class GPIO 
{
public:
    enum PinMode 
    {
        GPIO_INPUT,
        GPIO_OUTPUT,
        GPIO_INPUT_PULLUP,
        GPIO_PWM
    };
    
    PinMode getMode() const { return _current_mode; }
    GPIO(uint8_t pin);
    void setMode(PinMode mode);
    void write(bool state);
    bool read();
    void toggle();
    void setPWM(uint8_t duty);
    void attachInterrupt(void (*handler)(), int mode);

private:
    uint8_t _pin;
    PinMode _current_mode;
};