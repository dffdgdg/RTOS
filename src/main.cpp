#include "kernel/kernel.h"
#include "fs/fs.h"
#include "fs/logger.h"
#include "syscalls/syscalls.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "system/monitor.h"

int sem_test;
int counter = 0;
const int ledPin = 13;

Logger logger;
Timer sysTimer;
GPIO led(ledPin);

void blinkTask();
void counterTask();
void fsTask();
void systemMonitorTask();
void ledStatusTask(); 

void setup() 
{
    Serial.begin(9600);
    while(!Serial) {}
    delay(100);
    Serial.flush();
    sysTimer.begin();
    logger.begin();
    
    led.setMode(GPIO::GPIO_OUTPUT);
    
    if(!fs.createFile("counter.txt", "0")) 
    {
        logger.log("ERR: Failed to create counter.txt");
    }
    
    if(!fs.createFile("config.txt", "interval=1000")) 
    {
        logger.log("ERR: Failed to create config.txt");
    }

    kernel.addTask(counterTask,       1000,  0);
    kernel.addTask(ledStatusTask,         2000,  2); 
    kernel.addTask(systemMonitorTask, 10000, 3); 
    //kernel.addTask(fsTask,        4000,  4); 
    kernel.addTask(blinkTask,        1000,  4); 
    
    
    SystemGuard::enable();
    noInterrupts();
    Serial.println(F("\nRTOS Started"));
    interrupts();
}

void loop() 
{
    kernel.run();
    SystemGuard::reset();
}

void counterTask() 
{
    counter = (counter + 1) % 1000;
    
    static int lastCounter = -1;
    if(counter != lastCounter) {
        if(!fs.writeFile("counter.txt", String(counter))) {
            logger.log("ERR: Failed to write counter");
        }
        lastCounter = counter;
    }
}

void fsTask() 
{
    static uint32_t lastCheck = 0;
    if(sysTimer.millis() - lastCheck > 30000) 
    { 
        lastCheck = sysTimer.millis();
        
        if(!fs.fileExists("counter.txt")) 
        {
            logger.log("WARN: counter.txt missing, recreating");
            fs.createFile("counter.txt", "0");
        }
        
        String config = fs.readFile("config.txt");
        Serial.print("Config check: "); Serial.println(config);
    }
}

void systemMonitorTask() {
    static uint32_t lastPrint = 0;
    if(sysTimer.millis() - lastPrint < 10000) return;
    lastPrint = sysTimer.millis();
    
    Serial.print(F("\nStat: T="));
    Serial.print(kernel.getTaskCount());
    Serial.print(F(" F="));
    Serial.print(fs.getFileCount());
    Serial.print(F(" M="));
    Serial.print(SystemMonitor::freeMemory());
    Serial.println(F("B"));
}

void blinkTask() 
{
    if(led.getMode() != GPIO::GPIO_OUTPUT) 
    {
        led.setMode(GPIO::GPIO_OUTPUT);
    }
    led.toggle();
}

void ledStatusTask()
{
    static uint32_t lastPrintTime = 0;
    if(sysTimer.millis() - lastPrintTime >= 1000) 
    {
        lastPrintTime = sysTimer.millis();
        Serial.print("[");
        Serial.print(sysTimer.millis());
        Serial.print(" ms] Counter: ");
        Serial.println(counter);
    }
}