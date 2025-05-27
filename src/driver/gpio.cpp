#include "gpio.h"

/**
 * @brief Конструктор
 * @param pin Номер пина (0-13)
 */
GPIO::GPIO(uint8_t pin) : _pin(pin), _current_mode(GPIO_INPUT) 
{
    setMode(GPIO_INPUT);
}

/**
 * @brief Устанавливает режим работы пина
 * @param mode Режим работы (ввод/вывод/PWM/подтяжка)
 */
void GPIO::setMode(PinMode mode) 
{
    if(_pin > 13) return;
    
    _current_mode = mode;
    switch(mode) 
    {
        case GPIO_INPUT:
            pinMode(_pin, INPUT);
            break;
        case GPIO_OUTPUT:
            pinMode(_pin, OUTPUT);
            break;
        case GPIO_INPUT_PULLUP:
            pinMode(_pin, INPUT_PULLUP);
            break;
        case GPIO_PWM:
            pinMode(_pin, OUTPUT);
            break;
    }
}

/**
 * @brief Устанавливает состояние выхода (HIGH/LOW)
 * @param state Состояние выхода
 */
void GPIO::write(bool state) 
{
    if(_current_mode != GPIO_OUTPUT) return;
    if(_pin < 8) 
    {
        if(state) PORTD |= (1 << _pin);
        else PORTD &= ~(1 << _pin);
    } 
    else 
    {
        if(state) PORTB |= (1 << (_pin - 8));
        else PORTB &= ~(1 << (_pin - 8));
    }
}

/**
 * @brief Читает состояние входа
 * @return Состояние входа (HIGH/LOW)
 */
bool GPIO::read() 
{
    return digitalRead(_pin);
}

/**
 * @brief Переключает состояние выхода
 */
void GPIO::toggle() 
{
    if (_current_mode != GPIO_OUTPUT) return;
    write(!read());
}

/**
 * @brief Устанавливает PWM сигнал
 * @param duty Коэффициент заполнения (0-255)
 */
void GPIO::setPWM(uint8_t duty) 
{
    if(_current_mode != GPIO_PWM) return; 
    analogWrite(_pin, duty);
}

/**
 * @brief Подключает обработчик прерывания
 * @param handler Функция-обработчик
 * @param mode Режим прерывания (RISING/FALLING/CHANGE)
 */
void GPIO::attachInterrupt(void (*handler)(), int mode) 
{
    if(_pin < 2 || _pin > 13) return;
    ::attachInterrupt(digitalPinToInterrupt(_pin), handler, mode);
}