#include "stm8s.h"
#include "main.h"
#include "input.h"
#include "led.h"
#include "eeprom.h"

#define EEPROM_BASE_ADDRESSS  0x4000
#define EEPROM_CHK_BYTE       0x4a

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
  if(getEepromByte(eeprom_chk_adr) != EEPROM_CHK_BYTE) {
    setEepromByte(eeprom_chk_adr,             EEPROM_CHK_BYTE);
    setEepromByte(eeprom_nite_mode_adr,       nightMode);
    setEepromByte(eeprom_day_brightness_adr,  dayBrightness);
    setEepromByte(eeprom_nite_brightness_adr, nightBrightness);
    setEepromByte(eeprom_threshold_adr,       nightlightThresh);
  }
  else {
    nightMode         = getEepromByte(eeprom_nite_mode_adr);
    dayBrightness     = getEepromByte(eeprom_day_brightness_adr);
    nightBrightness   = getEepromByte(eeprom_nite_brightness_adr);
    nightlightThresh  = getEepromByte(eeprom_threshold_adr);
  }
}
