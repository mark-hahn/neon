#ifndef __EEPROM__
#define __EEPROM__

#include "stm8s.h"
#include "main.h"

// addr is 0 to 639
void setEepromByte(u16 addr, u8 data);
u8   getEepromByte(u16 addr);

#endif