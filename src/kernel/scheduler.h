#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include <avr/wdt.h>

#define MAX_TASKS 8        
#define MAX_SEMAPHORES 5   
#define WDT_TIMEOUT WDTO_4S 

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
    static bool isEnabled() 
    {
        return (WDTCSR & _BV(WDIE)) != 0;
    }
    static void enable(uint8_t timeout = WDT_TIMEOUT) 
    {
        cli(); 
        wdt_reset(); 
        MCUSR &= ~(1 << WDRF); 
        WDTCSR |= (1 << WDCE) | (1 << WDE); 
        WDTCSR = (1 << WDIE) | timeout; 
        sei(); 
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
    
    /**
     * @brief Получение указателя на функцию задачи
     * @param index Индекс задачи
     * @return Указатель на функцию или nullptr при ошибке
     */
    TaskFunction getTaskFunction(uint8_t index) const 
    {
        if (index >= taskCount) return nullptr;
        return tasks[index].function;
    }
    void run();
    
    uint8_t getTaskCount() const;
    
    void emergencyDump(const char* reason);
    
    int sem_create(int initial_count);
    bool sem_wait(int sem_id);
    bool sem_signal(int sem_id);
    bool sem_delete(int sem_id);
    void begin();
};

extern Scheduler kernel; 

#endif