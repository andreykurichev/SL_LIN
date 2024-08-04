#include "action_led.h"
#include "sio.h"
#include "lin_processor.h"
#include "lawicel.h"
#include <stdarg.h>
#include "passive_timer.h"
namespace sio
{

  // TODO: нужно ли установить контакты ввода/вывода (PD0, PD1)? Мы полагаемся на настройку
  // загрузчик?

  // Размер очереди выходных байтов. Должно быть <= 128, чтобы избежать переполнения.
  // TODO: уменьшить размер буфера? Хватит ли нам оперативной памяти?
  // TODO: увеличить размер индекса до 16 бит и увеличить размер буфера до 200?
  static const uint8 kQueueTXSize = 120;
  static const uint8 kQueueSerialRXSize = 64;
  static uint8 bufferTX[kQueueTXSize];
  static uint8 bufferSerial[kQueueSerialRXSize];

  // Индекс самой старой записи в буфере TX.
  static uint8 start;
  // Количество байтов в очереди TX.
  static uint8 count;
  // Индекс самой старой записи в буфере TX.
  static volatile uint8 rx_buffer_head;
  // Количество байтов в очереди TX.
  static volatile uint8 rx_buffer_tail;

  // Светодиод FRAMES - мигает при обнаружении действительных кадров.
  static ActionLed frames_activity_led(PORTD, 7); // D7

  // Вызывающий должен убедиться, что count < kQueueTXSize перед вызовом этого.
  static void unsafe_enqueue(byte b)
  {
    // kQueueTXSize достаточно мал, чтобы не было переполнения.
    uint8 next = start + count;
    if (next >= kQueueTXSize)
    {
      next -= kQueueTXSize;
    }
    bufferTX[next] = b;
    count++;
  }

  // Вызывающий должен убедиться, что count > 1 перед вызовом этого.
  static byte unsafe_dequeue()
  {
    const uint8 b = bufferTX[start];
    if (++start >= kQueueTXSize)
    {
      start = 0;
    }
    count--;
    return b;
  }

  void setup()
  {
    start = 0;
    count = 0;
    rx_buffer_head = 0;
    rx_buffer_tail = 0;
    lawicel::RX_Index = 0;

    frames_activity_led.action();

    // Девизы см. в таблице 19-12 в техническом описании atmega328p.
    // U2X0, 16 -> 115,2 кбод при 16 МГц.
    // U2X0, 207 -> 9600 бод @ 16 МГц.
    UBRR0H = 0;
    UBRR0L = 16; // Скорость uart
    // Бит U2X0 (1) регистра UCSR0A - удвоение скорости обмена, если установить в 1 (только в асинхронном режиме. в синхронном следует установить этот бит в 0).
    UCSR0A = H(U2X0);
    // Включаем приемник. Включить передатчик. Разрешаем прирывания.
    UCSR0B = H(RXEN0) | H(TXEN0) | H(RXCIE0);
    UCSR0C = H(UDORD0) | H(UCPHA0);
    sei(); // разрешение глобального прерывания
  }

  ISR(USART_RX_vect)
  {
    uint8 i = (rx_buffer_head + 1) % kQueueSerialRXSize;
    if (i != rx_buffer_tail)
    {
      bufferSerial[rx_buffer_head] = UDR0;
      rx_buffer_head = i;
    }
  }

  int available()
  {
    return (kQueueSerialRXSize + rx_buffer_head - rx_buffer_tail) % kQueueSerialRXSize;
  }

  char serial_read()
  {
    if (rx_buffer_head == rx_buffer_tail)
    {
      return 0;
    }
    if (rx_buffer_tail > kQueueSerialRXSize)
    {
      rx_buffer_tail = 0;
    }
    char serialChar = bufferSerial[rx_buffer_tail];
    rx_buffer_tail = (rx_buffer_tail + 1) % kQueueSerialRXSize;
    return serialChar;
  }

  void printchar(uint8 c)
  {
    // Если буфер заполнен, отбрасываем этот символ.
    // TODO: отбросить последний байт, чтобы освободить место для нового байта?
    if (count >= kQueueTXSize)
    {
      return;
    }
    unsafe_enqueue(c);
  }

  void loop()
  {
    frames_activity_led.loop();
    if (count && (UCSR0A & H(UDRE0)))
    {
      UDR0 = unsafe_dequeue();
    }
  }

  uint8 capacity()
  {
    return kQueueTXSize - count;
  }

  void waitUntilFlushed()
  {
    // Цикл занятости до тех пор, пока все не будет сброшено в UART.
    while (count)
    {
      loop();
    }
  }

  // Предположим, что n находится в диапазоне [0, 15].
  static void printHexDigit(uint8 n)
  {
    if (n < 10)
    {
      printchar((char)('0' + n));
    }
    else
    {
      printchar((char)(('a' - 10) + n));
    }
  }

  void printhex2(uint8 b)
  {
    printHexDigit(b >> 4);
    printHexDigit(b & 0xf);
  }

  void println()
  {
    printchar('\n');
  }

  extern void print_computer()
  {
    LinFrame frame;
    if (lin_processor::readNextFrame(&frame))
    {
      const boolean frameOk = frame.isValid();
      if (frameOk)
      {
        printchar('t');
        for (int i = 0; i < frame.num_bytes(); i++)
        {
          frames_activity_led.action();
          if (i == 1)
          {
            printchar(' ');
            printchar(lawicel::getDlc(frame.num_bytes()));
          }
          printhex2(frame.get_byte(i));
        }
        printchar(CR);
      }
    }
  }

  void print(const __FlashStringHelper *str)
  {
    const char *PROGMEM p = (const char PROGMEM *)str;
    for (;;)
    {
      const unsigned char c = pgm_read_byte(p++);
      if (!c)
      {
        return;
      }
      printchar(c);
    }
  }

  void println(const __FlashStringHelper *str)
  {
    print(str);
    println();
  }

  void print(const char *str)
  {
    for (;;)
    {
      const char c = *(str++);
      if (!c)
      {
        return;
      }
      printchar(c);
    }
  }

  void println(const char *str)
  {
    print(str);
    println();
  }

  void printf(const __FlashStringHelper *format, ...)
  {
    // Предполагается один поток, использующий статический буфер.
    static char buf[80];
    va_list ap;
    va_start(ap, format);
    vsnprintf_P(buf, sizeof(buf), (const char *)format, ap); // программа для AVR
    for (char *p = &buf[0]; *p; p++)                         // эмулировать приготовленный режим для новых строк
    {
      printchar(*p);
    }
    va_end(ap);
  }
} // пространство имен sio