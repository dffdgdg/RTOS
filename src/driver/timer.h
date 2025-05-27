#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer 
{
private:
    volatile uint32_t _millis;   
    uint32_t _last_tick;        
    bool _initialized;       
    
public:
    Timer() : _millis(0), _last_tick(0), _initialized(false) {}
    
    void begin();
    
    uint32_t millis() const;
    
    void delay(uint32_t ms);

    bool isInitialized() const { return _initialized; }
    
    void update();
};

extern Timer sysTimer;

#endif