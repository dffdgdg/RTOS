#include "kernel/kernel.h"
#include "fs/fs.h"
#include "fs/logger.h"
#include "syscalls/syscalls.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "system/monitor.h"
#include <LiquidCrystal.h>

int sem_test;
int counter = 0;

Logger logger;
Timer sysTimer;
GPIO led(13);
GPIO lcdRS(4);
GPIO lcdE(5);
GPIO lcdD4(6);
GPIO lcdD5(7);
GPIO lcdD6(8);
GPIO lcdD7(9);

LiquidCrystal lcd(
    lcdRS.getPin(), lcdE.getPin(), 
    lcdD4.getPin(), lcdD5.getPin(), 
    lcdD6.getPin(), lcdD7.getPin()
);

void blinkTask();
void counterTask();
void fsTask();
void systemMonitorTask();
void ledStatusTask(); 
void lcdTask(); 
void debugTime();
void testCrash();

void setup() 
{
    Serial.begin(9600);
    while (!Serial) {}
    delay(100);
    Serial.flush();
    
    sysTimer.begin();
    logger.begin();

    led.setMode(GPIO::GPIO_OUTPUT);
    lcdRS.setMode(GPIO::GPIO_OUTPUT);
    lcdE.setMode(GPIO::GPIO_OUTPUT);
    lcdD4.setMode(GPIO::GPIO_OUTPUT);
    lcdD5.setMode(GPIO::GPIO_OUTPUT);
    lcdD6.setMode(GPIO::GPIO_OUTPUT);
    lcdD7.setMode(GPIO::GPIO_OUTPUT);

    lcd.begin(16, 2);

    if (!fs.createFile("counter.txt", "0")) 
    {
        logger.log("ERR: Failed to create counter.txt");
    }

    if (!fs.createFile("config.txt", "interval=1000")) 
    {
        logger.log("ERR: Failed to create config.txt");
    }

    kernel.addTask(counterTask, 1000, 1);
    kernel.addTask(ledStatusTask, 2000, 2);
    kernel.addTask(systemMonitorTask, 10000, 3);
    kernel.addTask(fsTask, 4000, 4);
    kernel.addTask(blinkTask, 1000, 4);
    kernel.addTask(lcdTask, 5000, 4);
    //kernel.addTask(debugTime, 3000, 1);
    //kernel.addTask(testCrash, 3000, 1);

    if(SystemGuard::isEnabled()) 
    {
        SystemGuard::disable();
    }
    SystemGuard::enable(WDTO_8S);
    kernel.begin();
}

void testCrash() 
{
static int fileCounter = 0;
    String fileName = "hog" + String(fileCounter);
    String content(400, 'A'); 
    
    if (!fs.createFile(fileName, content)) 
    {
        logger.log("ERR: Failed to create " + fileName);
    } 
    else 
    {
        fileCounter++;
    }
    
    Serial.print("Free memory: ");
    Serial.println(SystemMonitor::freeMemory());
}

void loop() 
{
    
    SystemGuard::reset();
    kernel.run();
}

void counterTask() 
{
    counter = (counter + 1) % 1000;
    static int lastCounter = -1;

    if (counter != lastCounter) 
    {
        if (!fs.writeFile("counter.txt", String(counter))) 
        {
            logger.log("ERR: Failed to write counter");
        }
        lastCounter = counter;
    }
}

void fsTask() 
{
    static uint32_t lastCheck = 0;
    if (sysTimer.millis() - lastCheck > 30000) 
    {
        lastCheck = sysTimer.millis();

        if (!fs.fileExists("counter.txt")) 
        {
            logger.log("WARN: counter.txt missing, recreating");
            fs.createFile("counter.txt", "0");
        }

        String config = fs.readFile("config.txt");
        Serial.print("Config check: ");
        Serial.println(config);
    }
}

void debugTime() 
{
    while (true) {}
}

void systemMonitorTask() 
{
    static uint32_t lastPrint = 0;
    if (sysTimer.millis() - lastPrint < 10000) return;
    
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
    if (led.getMode() != GPIO::GPIO_OUTPUT) 
    {
        led.setMode(GPIO::GPIO_OUTPUT);
    }
    led.toggle();
}

void ledStatusTask() 
{
    static uint32_t lastPrintTime = 0;
    if (sysTimer.millis() - lastPrintTime >= 1000) {
        lastPrintTime = sysTimer.millis();
        Serial.print("[");
        Serial.print(sysTimer.millis());
        Serial.print(" ms] Counter: ");
        Serial.println(counter);
    }
}

void lcdTask() 
{
    static uint32_t lastUpdate = 0;
    if (sysTimer.millis() - lastUpdate >= 500) {
        lastUpdate = sysTimer.millis();

        lcd.setCursor(0, 0);
        lcd.print("s M:");
        lcd.print(SystemMonitor::freeMemory());
        lcd.print("B   "); 

        lcd.setCursor(0, 1);
        lcd.print("Counter: ");
        lcd.print(counter);
        lcd.print("    "); 
    }
}