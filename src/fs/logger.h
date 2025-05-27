#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger 
{
public:
    void begin();
    
    void log(const String& message);
};

extern Logger logger;

#endif