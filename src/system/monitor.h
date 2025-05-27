#ifndef MONITOR_H
#define MONITOR_H

#ifdef __AVR__
#include <Arduino.h>

namespace SystemMonitor 
{
    int freeMemory();
    int freeMemoryPercent();
    bool isMemoryCritical();
    float getVccVoltage();
    bool isLowVoltage();
    
    constexpr int TOTAL_MEMORY = 2048;
    constexpr float LOW_VOLTAGE_THRESHOLD = 3.3f;
}

#endif
#endif