#include "system/monitor.h"
#include <Arduino.h>

#ifdef __AVR__

extern int __heap_start, *__brkval;

int SystemMonitor::freeMemory() 
{
    int free_memory;
    if (__brkval == 0) 
    {
        free_memory = ((int)&free_memory) - ((int)&__heap_start);
    } 
    else 
    {
        free_memory = ((int)&free_memory) - ((int)__brkval);
    }
    return free_memory;
}

int SystemMonitor::freeMemoryPercent() 
{
    int free_mem = freeMemory();
    return (free_mem > 0) ? (free_mem * 100) / TOTAL_MEMORY : 0;
}

bool SystemMonitor::isMemoryCritical() 
{
    return freeMemoryPercent() < 5;
}

float SystemMonitor::getVccVoltage() 
{
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(2);
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC));
    uint8_t low = ADCL;
    unsigned int val = (ADCH << 8) | low;
    return (val * 1.1f) / 1024.0f;
}

bool SystemMonitor::isLowVoltage() 
{
    return getVccVoltage() < LOW_VOLTAGE_THRESHOLD;
}
#endif