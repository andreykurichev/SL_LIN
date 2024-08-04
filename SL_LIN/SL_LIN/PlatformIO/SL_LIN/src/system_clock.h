#ifndef SYS_CLOCK_H
#define SYS_CLOCK_H

#include <arduino.h>
#include "hardware_clock.h"

// Использует аппаратные часы для обеспечения 32-битного миллисекундного времени с момента запуска программы.
// Время цикла 32 миллисекунды составляет около 54 дней цикла.
namespace system_clock {
// Вызов один раз для основного цикла(). Обновляет внутренние часы миллисекунд на основе аппаратного обеспечения.
// Часы. Интервал вызова более 280 мс приведет к потере времени из-за
// к переполнению аппаратных часов.
extern void loop();

// Возвращаем время последнего обновления() в миллисекундах с момента запуска программы. Возвращает ноль, если update() был
// никогда не вызывается.
extern uint32 timeMillis();

}  // пространство имен system_clock

#endif
