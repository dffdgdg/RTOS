#include "logger.h"
#include "driver/timer.h"
#include "fs/fs.h"

/**
 * @brief Инициализация логгера
 */
void Logger::begin() 
{
    fs.createFile("log.txt", "");
}

/**
 * @brief Запись сообщения в лог
 * @param message Сообщение для записи
 */
void Logger::log(const String& message) 
{
    String timestamp = "[" + String(sysTimer.millis()) + " ms] ";
    String entry = timestamp + message + "\n";

    Serial.print(entry);
    
    String oldLog = fs.readFile("log.txt");

    if (oldLog.length() + entry.length() > 1024) 
    {
        oldLog = oldLog.substring(entry.length());  
    }

    fs.writeFile("log.txt", oldLog + entry);
}