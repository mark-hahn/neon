#include "stm8s.h"
#include "main.h"
#include "input.h"
#include "led.h"
#include "eeprom.h"

#define EEPROM_BASE_ADDRESSS  0x4000

// addr is 0 to 639
void setEepromByte(u16 addr, u8 data) {
  // Check if the EEPROM is write-protected
  // If it is then unlock it
  if (FLASH->IAPSR |= FLASH_IAPSR_DUL) {
    FLASH->DUKR = 0xAE;
    FLASH->DUKR = 0x56;
  }
  // Write the data to the EEPROM.
  (*(uint8_t *) (EEPROM_BASE_ADDRESSS + addr)) = data;
  
  // write protect the EEPROM.
  FLASH->IAPSR &= !FLASH_IAPSR_DUL;
}

// addr is 0 to 639
u8 getEepromByte(u16 addr) {
  return (*(uint8_t *) (EEPROM_BASE_ADDRESSS + addr)) ;
}

void initEeprom(void) {
  if(getEepromByte(eeprom_chk_adr) != 0x5b) {
    setEepromByte(eeprom_mode_adr,        mode);
    setEepromByte(eeprom_brightness_adr,  brightness);
    setEepromByte(eeprom_chk_adr,         0x5b);
  }
  else {
    mode       = getEepromByte(eeprom_mode_adr);
    brightness = getEepromByte(eeprom_brightness_adr);
  }
}
