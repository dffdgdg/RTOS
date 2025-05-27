#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include <avr/wdt.h>

#define MAX_TASKS 8        
#define MAX_SEMAPHORES 5   
#define WDT_TIMEOUT WDTO_2S // Таймаут watchdog-таймера (2 сек)

typedef void (*TaskFunction)();


struct Semaphore 
{
    int count;                    
    TaskFunction waiting[MAX_TASKS];
    uint8_t waitCount;              
};


struct Task 
{
    TaskFunction function;   
    unsigned long period;      
    unsigned long lastRun;     
    bool enabled;             
    uint8_t priority;          
    uint32_t runCount;         
    uint32_t maxRunTime;        
    uint32_t lastRunTime;     
};


class SystemGuard 
{
public:
    static void enable(uint8_t timeout = WDT_TIMEOUT) 
    {
        wdt_enable(timeout);
    }
    
    static void reset() 
    {
        wdt_reset();
    }
    
    static void disable() 
    {
        wdt_disable();
    }
};


class Scheduler 
{
private:
    Task tasks[MAX_TASKS];        
    uint8_t taskCount = 0;          
    Semaphore semaphores[MAX_SEMAPHORES];
    uint8_t semCount = 0;          
    
    int findTask(TaskFunction function) const;
    
    void sortTasks();
    
    void checkTaskTimings();

public:
    bool addTask(TaskFunction function, unsigned long period, uint8_t priority = 0);
    
    bool removeTask(TaskFunction function);
    
    bool enableTask(TaskFunction function, bool state);
    
    bool setPeriod(TaskFunction function, unsigned long new_period);
    
    bool setPriority(TaskFunction function, uint8_t new_priority);
    
    uint8_t getPriority(TaskFunction function) const;
    
    void run();
    
    uint8_t getTaskCount() const;
    
    void emergencyDump(const char* reason);
    
    int sem_create(int initial_count);
    bool sem_wait(int sem_id);
    bool sem_signal(int sem_id);
    bool sem_delete(int sem_id);
};

extern Scheduler kernel; 

#endif