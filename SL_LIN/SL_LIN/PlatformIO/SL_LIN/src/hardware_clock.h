#ifndef HARDWARE_CLOCK_H
#define HARDWARE_CLOCK_H

#include <arduino.h>
#include "avr_util.h"

// Предоставляет свободно работающий 16-битный счетчик с частотой 250 тактов в миллисекунду и
// время цикла около 280 миллисекунд. Предполагая тактовую частоту 16 МГц.
//
// ИСПОЛЬЗУЕТ: таймер 1, без прерываний.
namespace hardware_clock {
// Вызываем один раз из main setup(). Счетчик тиков начинается с 0.
extern void setup();

// Свободно работающий 16-битный счетчик. Начинает считать с нуля и зацикливается
// каждые ~280 мс.
// Предполагается, что прерывания разрешены при входе.
// НЕ ВЫЗЫВАЙТЕ ЭТО ИЗ ISR.
inline uint16 ticksForNonIsr() {
  // Мы отключаем прерывание, чтобы избежать повреждения буфера временных байтов AVR
  // который используется для чтения 16-битных значений.
  // TODO: можем ли мы избежать отключения прерываний (мотивация: улучшить джиттер LIN ISR).
  cli();
  const uint16 result TCNT1;
  sei();
  return result;
}

// Аналогичен ticksNonIsr, но не разрешает прерывания.
// ВЫЗОВАТЬ ЭТО ТОЛЬКО ИЗ ISR.
inline uint16 ticksForIsr() {
  return TCNT1;
}

#if F_CPU != 16000000
#error "The existing code assumes 16Mhz CPU clk."
#endif

// @ 16Mhz / x64 прескалер. Количество тактов в миллисекунду.
const uint32 kTicksPerMilli = 250;
}  // пространство имен hardware_clock

#endif
