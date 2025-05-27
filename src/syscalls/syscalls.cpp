#include "syscalls.h"
#include <Arduino.h>

namespace os 
{
    /**
     * @brief Создание задачи
     * @param taskFunc Функция задачи
     * @param period Период выполнения (мс)
     */
    void task_create(void (*taskFunc)(), unsigned long period) 
    {
        kernel.addTask(taskFunc, period);
    }

    /**
     * @brief Задержка выполнения задачи
     * @param ms Время задержки (мс)
     */
    void task_delay(unsigned long ms) 
    {
        unsigned long start = millis();
        while (millis() - start < ms) 
        {
            kernel.run();
        }
    }

    /**
     * @brief Удаление задачи
     * @param taskFunc Функция задачи для удаления
     */
    void task_delete(void (*taskFunc)()) 
    {
        kernel.removeTask(taskFunc);
    }

    /**
     * @brief Проверка существования файла
     * @param name Имя файла
     * @return true если файл существует
     */
    bool file_exists(const String& name) 
    {
        return fs.fileExists(name);
    }

    /**
     * @brief Чтение текстового файла
     * @param name Имя файла
     * @return Содержимое файла
     */
    String file_read(const String& name) 
    {
        return fs.readFile(name);
    }

    /**
     * @brief Запись в текстовый файл
     * @param name Имя файла
     * @param content Содержимое для записи
     * @return true если запись успешна
     */
    bool file_write(const String& name, const String& content) 
    {
        return fs.writeFile(name, content);
    }

    /**
     * @brief Удаление файла
     * @param name Имя файла
     * @return true если удаление успешно
     */
    bool file_delete(const String& name) 
    {
        return fs.deleteFile(name);
    }

    /**
     * @brief Перезагрузка системы
     */
    void sys_reboot() 
    {
        asm volatile ("jmp 0");
    }

    /**
     * @brief Получение информации о системе
     * @return Строка с информацией
     */
    String sys_info() 
    {
        String info;
        info += "OS v1.0\n";
        info += "Tasks: ";
        info += kernel.getTaskCount();
        info += "\nFiles: ";
        info += fs.getFileCount();
        return info;
    }
    
    /**
     * @brief Создание семафора
     * @param initial_count Начальное значение
     * @return Идентификатор семафора или -1 при ошибке
     */
    int sem_create(int initial_count) 
    {
        return kernel.sem_create(initial_count);
    }
    
    /**
     * @brief Ожидание семафора
     * @param sem_id Идентификатор семафора
     * @return true если семафор получен
     */
    bool sem_wait(int sem_id) 
    {
        return kernel.sem_wait(sem_id);
    }
    
    /**
     * @brief Освобождение семафора
     * @param sem_id Идентификатор семафора
     * @return true если операция успешна
     */
    bool sem_signal(int sem_id)
    {
        return kernel.sem_signal(sem_id);
    }
    
    /**
     * @brief Удаление семафора
     * @param sem_id Идентификатор семафора
     * @return true если удаление успешно
     */
    bool sem_delete(int sem_id) 
    {
        return kernel.sem_delete(sem_id);
    }
};