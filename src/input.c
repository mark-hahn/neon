#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "eeprom.h"
#include "animation.h"
#include "input.h"
#include "led.h"

#define EEPROM_CHK_BYTE 0x5a

// operation modes
enum {
  modeNormal,
  modeNightLight,
  modeAnim,
  modeSettingAnim,
  modeSettingSpeed
}

u8 mode = modeNormal;

// eeprom addresses
enum {
  eeprom_chk_adr,
  eeprom_brightness_adr,
  eeprom_mode_adr,
  eeprom_anim_adr,
  eeprom_speed_adr
};

void eepromInit() {
  if(getEepromByte(eeprom_chk_adr) != 0x5a) {
    setEepromByte(eeprom_brightness_adr,  (brightness - MIN_BRIGHTNESS));
    setEepromByte(eeprom_mode_adr,        mode);
    setEepromByte(eeprom_anim_adr,        animation);
    setEepromByte(eeprom_speed_adr,       animSpeed);
    setEepromByte(eeprom_chk_adr,         0x5a);
  }
  else {
    brightness = (getEepromByte(eeprom_brightness_adr) + MIN_BRIGHTNESS);
    mode       =  getEepromByte(eeprom_mode_adr);
    animation  =  getEepromByte(eeprom_anim_adr);
    animSpeed  =  getEepromByte(eeprom_speed_adr);
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

u8 clickCount = 0;

void clickTimeout() {
  if(clickCount == 1 && 
       mode != modeSettingAnim && 
       mode != modeSettingSpeed) {
    powerDown();  
    return;
  }
  if(clickCount >= 3) {
    mode = modeSettingAnim;   
    setEepromByte(eeprom_mode_adr,        mode);
    return;
  }
  switch(mode) {
    case modeNormal:     
      // clickCount == 2
      mode = modeNightLight;
      setEepromByte(eeprom_mode_adr, mode);
      flash(2); 
      break;

    case modeNightLight: 
      // clickCount == 2
      mode = modeAnim;
      setEepromByte(eeprom_mode_adr, mode);
      flash(3);
      break;

    case modeAnim: 
      // clickCount == 2
      stopAnimation(); 
      mode = modeNormal;   
      setEepromByte(eeprom_mode_adr, mode);
      flash(1); 
      break;

    case modeSettingAnim:  
      mode = modeSettingSpeed; 
      setEepromByte(eeprom_mode_adr, mode);
      break;
    case modeSettingSpeed: 
      mode = modeAnim;
      setEepromByte(eeprom_mode_adr, mode);
      break;
  }
  clickCount = 0;
}

void adjBrightness(bool cw) {
  if( cw && brightness < MAX_BRIGHTNESS) {
    brightness++;
    setEepromByte(eeprom_brightness_adr,  (brightness - MIN_BRIGHTNESS));
  }
  if(!cw && brightness > MIN_BRIGHTNESS) {
    brightness--;
    setEepromByte(eeprom_brightness_adr,  (brightness - MIN_BRIGHTNESS));
  }
}

void chgAnim(bool cw) {
  if( cw && ++anim == numAnims) {
    animation = 0;
    setEepromByte(eeprom_anim_adr, animation);
  }
  if(!cw && --anim < 0) {
    animation = numAnims-1;
    setEepromByte(eeprom_anim_adr, animation);
  }
}

void adjSpeed(bool cw) {
  if( cw && animSpeed < MAX_SPEED) {
    animSpeed++;
    setEepromByte(eeprom_speed_adr, animSpeed);
  }
  if(!cw && animSpeed > 0) {
    animSpeed--;
    setEepromByte(eeprom_speed_adr, animSpeed);
  }
}

u16 lastClickTime = 0;

void buttonPress() {
  lastClickTime = millis();
  clickCount++;
}

void encoderTurn(bool cw) {
  switch(mode) {
    case modeSettingAnim:  chgAnim(cw);       break;
    case modeSettingSpeed: adjSpeed(cw);      break;
    default:               adjBrightness(cw); break;
  }
}

#define DEBOUNCE_DELAY_MS 10

u16 lastBtnActivity = 0;

// irq6 interrupts every button pin rising edge (port D)
@far @interrupt void buttonIntHandler() {
  static bool btnWaitDebounce  = false;
  // no change when button is pressed to turn on power
  u16 now = millis();
  // check button if no activity for 10ms
  if(btnWaitDebounce && ((now - lastBtnActivity) > DEBOUNCE_DELAY_MS)
    btnWaitDebounce = false;

  if(!btnWaitDebounce) {
    // level should always be high since only interrupts on rising edge
    buttonPress();
    lastBtnActivity = now;
    btnWaitDebounce = true;
  }
}

// irq5 interrupts either encoder pin rising edge (port C)
@far @interrupt void encoderIntHandler() {
  static bool encaWaitDebounce = false;
  static u16  lastEncaActivity = 0;
  static bool lastEncaWasDown  = false;

  u16 now = millis(void);
  if(encaWaitDebounce && ((now - lastEncaActivity) > DEBOUNCE_DELAY_MS)
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

  // button and encoders are external interrupts
  button_in_int;
  enca_in_int;
  encb_in_int;

  // all ports interrupt on rising edge only
  EXTI_CR1 = 0b01010101;

  flash(1);  // indicate mode is normal
}

#define CLICK_DELAY     300  // 300 ms  timeout for counting clicks
#define SETTING_DELAY 30000  //  30 sec timeout while setting

// called every timer interrupt (64 usecs) from led.c
// runs at highest interrupt priority
void inputLoop(void) {
  u16 now = millis();

  // click delay timeout starts on button release
  if(button_lvl) lastBtnActivity = now;

  if(clickCount > 0 && ((now - lastClickTime) > CLICK_DELAY) 
    clickTimeout();

  // check for timeout while setting anim or speed
  if((mode == modeSettingAnim || mode == modeSettingSpeed) 
      && ((now - lastClickTime) > SETTING_DELAY)) {
    mode = modeAnim;
    setEepromByte(eeprom_mode_adr, mode);
  }

  if(mode == modeAnim || 
     mode == modeSettingAnim ||
     mode == modeSettingSpeed) 
    doAnim();
}

