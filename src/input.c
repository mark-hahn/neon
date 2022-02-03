#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "eeprom.h"
#include "animation.h"
#include "input.h"
#include "led.h"

// brightness (0..7)  is 2^^(brightness-1) ma,  1/2..64 ma
#define BRIGHTNESS_MAX 7 

// speed (0..10) is 2^^(speed-4) secs,  1/16..64 secs per action
#define SPEED_MAX      10 

#define EEPROM_CHK_BYTE 0x5a

// operation modes
enum {
  modeNormal,
  modeNightLight,
  modeAnim
}

// setting state of control
enum {
  stateNotSetting,   // not changing mode/speed, knob sets brightness
  stateSettingAnim,  // setting mode  with knob
  stateSettingSpeed  // setting speed with knob
}

// eeprom addresses
enum {
  eeprom_chk_adr,
  eeprom_brightness_adr,
  eeprom_mode_adr,
  eeprom_anim_adr,
  eeprom_speed_adr
};

u8 settingState = stateNotSetting;
u8 mode         = modeNormal;

void eepromInit() {
  if(getEepromByte(eeprom_chk_adr) != 0x5a) {
    setEepromByte(eeprom_brightness_adr,  brightness);
    setEepromByte(eeprom_mode_adr,        mode);
    setEepromByte(eeprom_anim_adr,        animation);
    setEepromByte(eeprom_speed_adr,       animSpeed);
    setEepromByte(eeprom_chk_adr,         0x5a);
  }
  else {
    brightness = getEepromByte(eeprom_brightness_adr);
    mode       = getEepromByte(eeprom_mode_adr);
    animation  = getEepromByte(eeprom_anim_adr);
    animSpeed  = getEepromByte(eeprom_speed_adr);
  }
}

// set pwron gpio pin to zero
// this turns off 3.3v power to mcu
// this never returns;
void powerDown() {
  ints_off;
  pwron_clr;
  while(true);
}

// button click
void buttonPress(u8 clickCount) {
  switch(settingState) {
    case stateNotSetting: {
      if(clickCount == 1) powerDown();  return;
      if(clickCount == 2) {
        switch(mode) {
          case modeNormal:     mode = modeNightLight; flash(2); break;
          case modeNightLight: mode = modeAnim;       flash(3); break;
          case modeAnim: 
            // inputLoop might run doAnim
            ints_off;
            resetAnim(); 
            mode = modeNormal;   
            ints_on;
            flash(1); 
            break;
        }
      }
    }
    break;
    case stateSettingAnim:  settingState = stateSettingSpeed; break;
    case stateSettingSpeed: settingState = stateNotSetting;   break;
  }
}

void adjBrightness(bool cw) {
  if( cw && brightness < BRIGHTNESS_MAX) brightness++;
  if(!cw && brightness > 0)              brightness--;
}

void chgAnim(bool cw) {
  if( cw && ++anim == numAnims) animation = 0;
  if(!cw && --anim < 0)         animation = numAnims-1;
  resetAnim();
}

void adjSpeed(bool cw) {
  if( cw && animSpeed < SPEED_MAX) animSpeed++;
  if(!cw && animSpeed > 0)         animSpeed--;
}

void encoderTurn(bool cw) {
  switch(settingState) {
    case stateNotSetting:   adjBrightness(cw); break;
    case stateSettingAnim:  chgAnim(cw);       break;
    case stateSettingSpeed: adjSpeed(cw);      break;
  }
}

#define DEBOUNCE_DELAY_MS 10

// irq6 interrupts every button or encoder pin change (port D)
@far @interrupt void buttonIntHandler() {
  u16 now = millis();
  // check button if no activity for 10ms
  if(btnWaitDebounce && ((now - lastBtnActivity) > DEBOUNCE_DELAY_MS)
    btnWaitDebounce = false;

  if(!btnWaitDebounce) {
    btnDown = button_lvl; 
    if(btnDown != lastBtnWasDown) {
      if(btnDown) buttonPress();
      lastBtnWasDown  = btnDown;
      lastBtnActivity = now;
      btnWaitDebounce = true;
    }
  }
}

// irq5 interrupts every encoder pin change (port C)
@far @interrupt void encoderIntHandler() {
  u16 now = millis();
  if(encaWaitDebounce && ((now - lastEncaActivity) > BUTTON_DEBOUNCE_DELAY_MS)
    encaWaitDebounce = false;

  if(!encaWaitDebounce) {
    encaDown = enca_lvl; 
    if(encaDown != lastEncaWasDown) {
      if(encb_lvl) encoderTurn(true);
      else         encoderTurn(false);
      lastEncaWasDown  = encaDown;
      lastEncaActivity = now;
      encaWaitDebounce = true;
    }
  }
}

void initInput(void) {
  eepromInit();

  // set button (IRQ6)  interrupt priority to 1 (lowest)
  ITC->ISPR2 = (ITC->ISPR2 & ~0x30) | 0x10; // I1I0 is 0b01 

  // set encoder (IRQ5) interrupt priority to 2 (middle)
  ITC->ISPR2 = (ITC->ISPR2 & ~0x0c) | 0x00; // I1I0 is 0b00

  // button and encoder external interrupts
  button_in_int;
  enca_in_int;
  encb_in_int;

  // pins interrupt on both edges
  EXTI_CR1 = 0xff;

  flash(1);  // indicate mode is normal
}

// called every timer interrupt (64 usecs) from led.c
// runs at highest interrupt priority
void inputLoop(void) {
  if(mode == modeAnim) doAnim();
}

