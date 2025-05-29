#include "timer.h"
#include "kernel/scheduler.h"
#include <Arduino.h>

// Обработчик прерывания таймера
ISR(TIMER1_COMPA_vect, ISR_NOBLOCK) 
{
    sysTimer.update();
}

/**
 * @brief Инициализация системного таймера
 */
void Timer::begin() 
{
    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 1999;  // 16MHz / (8 * 1000Hz) - 1 = 1999
    TCCR1B |= (1 << WGM12);  // Режим CTC
    TCCR1B |= (1 << CS11);   // Предделитель = 8 (вместо CS10)
    TIMSK1 |= (1 << OCIE1A); // Разрешить прерывание
    _millis = 0;
    _last_tick = 0;
    interrupts();
}

/**
 * @brief Обновление счетчика времени
 */
void Timer::update() 
{
    _millis++;
}

/**
 * @brief Получить текущее время
 * @return Количество миллисекунд с начала работы
 */
uint32_t Timer::millis() const 
{
    uint32_t m;
    noInterrupts();
    m = _millis;
    interrupts();
    return m;
}

/**
 * @brief Задержка
 * @param ms Время задержки в миллисекундах
 */
void Timer::delay(uint32_t ms) 
{
    uint32_t start = _millis;
    while (_millis - start < ms) 
    {
        kernel.run();
    }
}
