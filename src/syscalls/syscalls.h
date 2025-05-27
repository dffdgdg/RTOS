#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kernel/kernel.h"
#include "fs/fs.h"

namespace os 
{
    void task_create(void (*taskFunc)(), unsigned long period);
    void task_delay(unsigned long ms);
    void task_delete(void (*taskFunc)());
    bool file_exists(const String& name);
    String file_read(const String& name);
    bool file_write(const String& name, const String& content);
    bool file_delete(const String& name);
    void sys_reboot();
    String sys_info();
    
    int sem_create(int initial_count = 1);
    bool sem_wait(int sem_id);
    bool sem_signal(int sem_id);
    bool sem_delete(int sem_id);
};

#endif