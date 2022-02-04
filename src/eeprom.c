#include "stm8s.h"
#include "main.h"

#define EEPROM_BASE_ADDRESSS  0x4000

// addr is 0 to 639
void setEepromByte(u16 addr, u8 data) {
  // Check if the EEPROM is write-protected
  // If it is then unlock it
  if (FLASH->PUKR == 0) {
    FLASH->DUKR = 0xAE;
    FLASH->DUKR = 0x56;
  }
  // Write the data to the EEPROM.
  (*(uint8_t *) (EEPROM_BASE_ADDRESSS + addr)) = data;
  
  // write protect the EEPROM.
  FLASH->PUKR = 0;
}

// addr is 0 to 639
u8 getEepromByte(u16 addr) {
  return (*(uint8_t *) (EEPROM_BASE_ADDRESSS + addr)) ;
}
