#ifndef __EEPROM__
#define __EEPROM__

#include "stm8s.h"
#include "main.h"

// eeprom addresses
enum {
  eeprom_chk_adr,
  eeprom_nite_mode_adr,
  eeprom_day_brightness_adr,
  eeprom_nite_brightness_adr,
  eeprom_threshold_adr
};

// addr is 0 to 639
void setEepromByte(u16 addr, u8 data);
u8   getEepromByte(u16 addr);

void initEeprom(void);

#endif