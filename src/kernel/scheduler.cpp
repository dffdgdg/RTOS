#include "scheduler.h"
#include "driver/timer.h"
#include "fs/logger.h"

extern Logger logger;
Scheduler kernel;

/**
 * @brief Аварийный дамп системы
 * @param reason Причина аварии
 */
void Scheduler::emergencyDump(const char* reason) 
{
    SystemGuard::disable();
    noInterrupts();
    
    Serial.println("\n=== SYSTEM DUMP ===");
    Serial.print("Reason: "); Serial.println(reason);
    Serial.print("Uptime: "); Serial.print(sysTimer.millis()); Serial.println(" ms");
    Serial.print("Tasks: "); Serial.println(taskCount);
    
    for(int i = 0; i < taskCount; i++) 
    {
        Serial.print("Task "); Serial.print(i);
        Serial.print(": runs="); Serial.println(tasks[i].runCount);
    }
    
    Serial.println("Rebooting...");
    delay(100);
    asm volatile ("jmp 0"); 
}

/**
 * @brief Поиск задачи по функции
 * @param function Указатель на функцию задачи
 * @return Индекс задачи или -1 если не найдена
 */
int Scheduler::findTask(TaskFunction function) const 
{
    for(int i = 0; i < taskCount; i++) 
    {
        if(tasks[i].function == function) 
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Добавление новой задачи
 * @param function Функция задачи
 * @param period Период выполнения (мс)
 * @param priority Приоритет (0 - высший)
 * @return true если задача добавлена успешно
 */
bool Scheduler::addTask(TaskFunction function, unsigned long period, uint8_t priority) 
{
    if(taskCount >= MAX_TASKS || period == 0 || priority > 255) 
    {
        logger.log("ERR: Can't add task");
        return false;
    }
    
    if(findTask(function) != -1) 
    {
        logger.log("ERR: Task exists");
        return false;
    }
    
    tasks[taskCount] = {function, period, 0, true, priority, 0, 0, 0};
    taskCount++;
    sortTasks(); 
    return true;
}

/**
 * @brief Сортировка задач по приоритету
 */
void Scheduler::sortTasks() 
{
    for(uint8_t i=1; i<taskCount; i++) 
    {
        Task key = tasks[i];
        int8_t j = i-1;
        while(j >= 0 && tasks[j].priority > key.priority) 
        {
            tasks[j+1] = tasks[j];
            j--;
        }
        tasks[j+1] = key;
    }
}

/**
 * @brief Основной цикл планировщика
 */
void Scheduler::run() 
{
    uint32_t now = sysTimer.millis();
    
    for(int i = 0; i < taskCount; i++) 
    {
        if(tasks[i].enabled && (now - tasks[i].lastRun >= tasks[i].period)) 
        {
            uint32_t startTime = now;
            tasks[i].lastRun = now;
            tasks[i].runCount++;
            
            tasks[i].function();
            
            uint32_t runTime = sysTimer.millis() - startTime;
            tasks[i].lastRunTime = runTime;
            if(runTime > tasks[i].maxRunTime) 
            {
                tasks[i].maxRunTime = runTime;
            }
        }
    }
    
    //checkTaskTimings(); 
}

/**
 * @brief Проверка временных характеристик задач
 */
void Scheduler::checkTaskTimings() 
{
    for(int i = 0; i < taskCount; i++) 
    {
        if(tasks[i].maxRunTime > tasks[i].period) 
        {
           // emergencyDump("Task overrun");
        }
    }
}

/**
 * @brief Получение количества задач
 * @return Количество активных задач
 */
uint8_t Scheduler::getTaskCount() const 
{
    return taskCount;
}

/**
 * @brief Создание семафора
 * @param initial_count Начальное значение счетчика
 * @return Идентификатор семафора или -1 при ошибке
 */
int Scheduler::sem_create(int initial_count) 
{
    if (semCount >= MAX_SEMAPHORES) return -1;
    
    semaphores[semCount].count = initial_count;
    semaphores[semCount].waitCount = 0;
    return semCount++;
}

/**
 * @brief Ожидание семафора
 * @param sem_id Идентификатор семафора
 * @return true если семафор получен
 */
bool Scheduler::sem_wait(int sem_id) 
{
    if (sem_id < 0 || sem_id >= semCount) return false;
    
    if (semaphores[sem_id].count > 0) 
    {
        semaphores[sem_id].count--;
        return true;
    } 
    else 
    {
        if (semaphores[sem_id].waitCount >= MAX_TASKS) return false;
        
        for (int i = 0; i < taskCount; i++) 
        {
            if (tasks[i].enabled) 
            {
                semaphores[sem_id].waiting[semaphores[sem_id].waitCount++] = tasks[i].function;
                tasks[i].enabled = false;
                return false;
            }
        }
        return false;
    }
}

/**
 * @brief Освобождение семафора
 * @param sem_id Идентификатор семафора
 * @return true если операция успешна
 */
bool Scheduler::sem_signal(int sem_id) 
{
    if (sem_id < 0 || sem_id >= semCount) return false;
    
    if (semaphores[sem_id].waitCount > 0) 
    {
        TaskFunction taskToWake = semaphores[sem_id].waiting[0];
        
        for (uint8_t i = 0; i < semaphores[sem_id].waitCount - 1; i++) 
        {
            semaphores[sem_id].waiting[i] = semaphores[sem_id].waiting[i+1];
        }
        semaphores[sem_id].waitCount--;
        
        for (int i = 0; i < taskCount; i++) 
        {
            if (tasks[i].function == taskToWake) 
            {
                tasks[i].enabled = true;
                tasks[i].lastRun = sysTimer.millis();
                break;
            }
        }
    } else 
    {
        semaphores[sem_id].count++;
    }
    return true;
}

/**
 * @brief Удаление семафора
 * @param sem_id Идентификатор семафора
 * @return true если удаление успешно
 */
bool Scheduler::sem_delete(int sem_id) 
{
    if (sem_id < 0 || sem_id >= semCount) return false;
    
    for (uint8_t i = sem_id; i < semCount - 1; i++) 
    {
        semaphores[i] = semaphores[i+1];
    }
    semCount--;
    return true;
}