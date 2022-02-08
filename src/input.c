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
};

// eeprom addresses
enum {
  eeprom_chk_adr,
  eeprom_brightness_adr,
  eeprom_mode_adr,
  eeprom_anim_adr,
  eeprom_speed_adr
};


#define DEFAULT_BRIGHTNESS 5
#define DEFAULT_ANIM_SPEED 5

u8 brightness = DEFAULT_BRIGHTNESS;
u8 mode       = modeNormal;
u8 animation  = 0;
u8 animSpeed  = DEFAULT_ANIM_SPEED;

void eepromInit(void) {
  if(getEepromByte(eeprom_chk_adr) != 0x5b) {
    setEepromByte(eeprom_brightness_adr, (brightness - MIN_BRIGHTNESS));
    setEepromByte(eeprom_mode_adr,        mode);
    setEepromByte(eeprom_anim_adr,        animation);
    setEepromByte(eeprom_speed_adr,       animSpeed);
    setEepromByte(eeprom_chk_adr,         0x5b);
  }
  else {
    brightness = (getEepromByte(eeprom_brightness_adr) + MIN_BRIGHTNESS);
    mode       =  getEepromByte(eeprom_mode_adr);
    animation  =  getEepromByte(eeprom_anim_adr);
    animSpeed  =  getEepromByte(eeprom_speed_adr);
  }
}

bool justPoweredOn = false;

// set pwron gpio pin to zero
// this turns off 3.3v power to mcu
// this never returns;
void powerDown() {
  pwron_clr;
  while(true);
}

u8 clickCount = 0;

void clickTimeout(void) {
  if(clickCount == 1) {  
    switch(mode) {
      case modeSettingAnim:
        // not setting eeprom
        mode = modeSettingSpeed; 
        break;
      case modeSettingSpeed:
        mode = modeAnim;
        setEepromByte(eeprom_mode_adr, mode);
        flash(mode);
        break;
      default: powerDown();  
    }
  }
  else if(clickCount >= 3) {
    // not setting eeprom
    mode       = modeSettingAnim; 
    brightness = DEFAULT_BRIGHTNESS;
    animSpeed  = DEFAULT_ANIM_SPEED;
  }
  else {
  // clickCount == 2
    switch(mode) {
      case modeNormal:     
        mode = modeNightLight;
        setEepromByte(eeprom_mode_adr, mode);
        flash(mode);
        break;

      case modeNightLight: 
        mode = modeAnim;
        setEepromByte(eeprom_mode_adr, mode);
        flash(mode);
        break;

      case modeAnim: 
        stopAnimation(); 
        mode = modeNormal;   
        setEepromByte(eeprom_mode_adr, mode);
        flash(mode);
        break;
    }
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
  if( cw && ++animation == numAnims) {
    animation = 0;
    setEepromByte(eeprom_anim_adr, animation);
  }
  if(!cw && --animation < 0) {
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

volatile u16 buttonPressCount = 0; // debug

void buttonPress(void) {
  lastClickTime = millis();
  clickCount++;

  buttonPressCount++; // debug
}

void encoderTurn(bool cw) {
  switch(mode) {
    case modeSettingAnim:  chgAnim(cw);       break;
    case modeSettingSpeed: adjSpeed(cw);      break;
    default:               adjBrightness(cw); break;
  }
}

#define DEBOUNCE_DELAY_MS 1000

u16 lastBtnActivity = 0;

// irq6 interrupt, button pin rising edge (port D)
@far @interrupt void buttonIntHandler() {
  static bool btnWaitDebounce  = false;
  u16 now = millis();

  // check button if no activity for 10ms
  if(btnWaitDebounce && ((now - lastBtnActivity) > DEBOUNCE_DELAY_MS))
    btnWaitDebounce = false;

  if(!btnWaitDebounce) {
    // level should always be high since only interrupts on rising edge
    buttonPress();
    lastBtnActivity = now;
    btnWaitDebounce = true;
  }
}

// irq5 interrupt, either encoder pin rising edge (port C)
@far @interrupt void encoderIntHandler() {
  static bool lastEncHigh      = true;
  static bool encaWaitDebounce = false;
  static u16  lastEncaActivity = 0;

  u16 now = millis();
  bool encHigh = (enca_lvl != 0);

  if(!encHigh || lastEncHigh) return;  // not A rising
  lastEncHigh = encHigh;

  if(encaWaitDebounce && ((now - lastEncaActivity) > DEBOUNCE_DELAY_MS))
    encaWaitDebounce = false;

  if(!encaWaitDebounce) {
    if(encb_lvl) encoderTurn(true);
    else         encoderTurn(false);
    lastEncaActivity = now;
    encaWaitDebounce = true;
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

  // all gpio ports interrupt on rising edge only
  EXTI->CR1 = 0x55;

  flash(1);  // indicate mode is normal
}

#define CLICK_DELAY     300  // 300 ms  timeout for counting clicks
#define SETTING_DELAY 30000  //  30 sec timeout while setting

// called every timer interrupt (64 usecs) from led.c
// runs at highest interrupt priority
void inputLoop(void) {
  u16 now = millis();

  if(justPoweredOn) {
    flash(mode);
    justPoweredOn = false;
  }

  // click delay timeout starts on button release
  if(button_lvl) lastBtnActivity = now;

  if(clickCount > 0 && ((now - lastClickTime) > CLICK_DELAY)) 
    clickTimeout();

  // check for timeout while setting anim or speed
  if((mode == modeSettingAnim || mode == modeSettingSpeed) 
      && ((now - lastClickTime) > SETTING_DELAY)) {
    mode = modeAnim;
    setEepromByte(eeprom_mode_adr, mode);
    flash(mode);
  }

  if(mode == modeAnim || 
     mode == modeSettingAnim ||
     mode == modeSettingSpeed) 
    doAnim();
}

