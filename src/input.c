#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "eeprom.h"

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

enum {
  blinkAnim,     // obnoxious blinking animation, instant rise/fall
  blinkSoftAnim, // blinking animation with rise/fall action speed
  // ... more animation modes
  numAnims
}

// setting state of control
enum {
  stateNotSetting,   // not changing mode/speed, knob sets brightness
  stateSettingAnim,  // setting mode  with knob
  settingSpeed  // setting speed with knob
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

i8 brightness   = BRIGHTNESS_MAX;
i8 anim         = 0;
i8 speed        = SPEED_MAX / 2;  // 2 secs per action

void eepromInit() {
  if(getEepromByte(eeprom_chk_adr) != 0x5a) {
    setEepromByte(eeprom_brightness_adr,  brightness);
    setEepromByte(eeprom_mode_adr,        mode);
    setEepromByte(eeprom_anim_adr,        anim);
    setEepromByte(eeprom_speed_adr,       speed);
    setEepromByte(eeprom_chk_adr,         0x5a);
  }
  else {
    brightness = getEepromByte(eeprom_brightness_adr);
    mode       = getEepromByte(eeprom_mode_adr);
    anim       = getEepromByte(eeprom_anim_adr);
    speed      = getEepromByte(eeprom_speed_adr);
  }
}

// button click (just pressed)
void buttonPress() {

}

void adjBrightness(i8 dir) {
  brightness += dir;       
  if(brightness == (BRIGHTNESS_MAX+1))
    brightness = 0;
  if(brightness == -1)
    brightness = BRIGHTNESS_MAX;
}

void chgAnim(i8 dir) {
  anim += dir;       
  if(anim == (numAnims)) anim = 0;
  if(anim == -1)         anim = numAnims-1;
}

void adjSpeed(i8 dir) {
  speed += dir;       
  if(speed == (SPEED_MAX+1))
    speed = 0;
  if(speed == -1)
    speed = SPEED_MAX;
}

// dir is +1 for CW  encoder click
// dir is -1 for CCW encoder click
void encoderTurn(i8 dir) {
  switch(settingState) {
    case stateNotSetting:  adjBrightness(dir); break;
    case stateSettingAnim: chgAnim(dir);       break;
    case settingSpeed:     adjSpeed(dir);      break;
  }
}

#define BUTTON_DEBOUNCE_DELAY_MS 10

// called from input interrupts or when pending
void inputHandler() {
  static inInputIntHandler   = false;
  static inputHandlerPending = false;

  // encoder interrupt (priority 2) might happen during handling of
  // button  interrupt (priority 1) and inputHandler is not re-entrant
  if(inInputIntHandler) {
    inputHandlerPending = true;
    return;
  } 
  else {
    static bool lastBtnWasDown  = false;
    static u16  lastBtnActivity = 0;
    static bool btnWaitDebounce = false;
    bool btnDown;

    static bool lastEncaWasDown  = false;
    static u16  lastEncaActivity = 0;
    static bool encaWaitDebounce = false;
    bool encaDown;

    inInputIntHandler = true;
    u16 now = millis();

    // check button if no activity for 10ms
    if(btnWaitDebounce && ((now - lastBtnActivity) > BUTTON_DEBOUNCE_DELAY_MS)
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

    // check encoder A if no activity for 10ms
    if(encaWaitDebounce && ((now - lastEncaActivity) > BUTTON_DEBOUNCE_DELAY_MS)
      encaWaitDebounce = false;

    if(!encaWaitDebounce) {
      encaDown = enca_lvl; 
      if(encaDown != lastEncaWasDown) {
        if(encb_lvl) encoderTurn(+1); // CW
        else         encoderTurn(-1); // CCW
        lastEncaWasDown  = encaDown;
        lastEncaActivity = now;
        encaWaitDebounce = true;
      }
    }
    inInputIntHandler = false;
  }
}

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void inputIntHandler() {
  inputHandler();
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
}

// called every timer interrupt (64 usecs) from led.c
// runs at highest interrupt priority
void inputLoop(void) {
  if(inputHandlerPending) {
    inputHandlerPending = false;
    inputIntHandler();
  }
}

