#ifndef LAWICEL_H
#define LAWICEL_H

#include <arduino.h>

#include "avr_util.h"

namespace lawicel
{
  extern bool isConnected;
  extern uint8 RX_Index;
  extern uint8 id;
  extern uint8 dlc;

#define CR '\r'
#define BEL 7

#define SERIAL_RESPONSE "N0001\r"
#define SW_VERSION_RESPONSE "v0107\r"
#define VERSION_RESPONSE "V1010\r"

  enum COMMAND : char
  {
    COMMAND_SET_BITRATE = 'S',    // установить битрейт LIN
    COMMAND_SET_BTR = 's',        // установить битрейт LIN через
    COMMAND_OPEN_CAN_CHAN = 'O',  // открыть LIN-канал
    COMMAND_CLOSE_CAN_CHAN = 'C', // закрыть LIN-канал
    COMMAND_SEND_11BIT_ID = 't',  // отправить LIN-сообщение с 11bit ID
    COMMAND_GET_VERSION = 'V',    // получить версию аппаратного и программного обеспечения
    COMMAND_GET_SW_VERSION = 'v', // получить только версию ПО
    COMMAND_GET_SERIAL = 'N',     // получить серийный номер устройства
    COMMAND_TIME_STAMP = 'Z',     // переключить настройку метки времени
  };


  extern void processChar(char rxChar);
  extern void process();
  extern void receiveCommand();
  extern void connectLin();
  extern void disconnectLin();
  extern void receiveSetBitrateCommand();
  extern void receiveTransmitCommand();
  extern void receiveTimestampCommand();
  extern void receiveSetBtrCommand();

  unsigned char hexCharToByte(char hex);
  extern unsigned char getDlc(uint8 n);

} // namespace lawicel

#endif