#include "Req_Modbus.h"
#include "Config.h"
#include <ModbusRTU.h>

#define SLAVE_ID1  1
#define FIRST_REG  0
#define REG_COUNT  2

ModbusRTU mb;

void modbusSetup() {
  Serial1.begin(9600, SERIAL_8N1, 27, 13);
  mb.begin(&Serial1);
  mb.master();
}

void modbusLoop() {
  static unsigned long lastPoll = 0;
  if (millis() - lastPoll < 1000) return;
  lastPoll = millis();

  uint16_t buf[REG_COUNT];

  if (!mb.slave()) {
    mb.readIreg(SLAVE_ID1, FIRST_REG, buf, REG_COUNT);
    while (mb.slave()) {
      mb.task();
    }
    pv1 = buf[0];
    sv1 = buf[1];
  }
}
