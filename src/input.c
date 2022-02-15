#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "eeprom.h"
#include "input.h"
#include "led.h"

// global
bool nightLightMode = false;

// todo - measure this
u16 nightlightThresh = 500;

// set pwron gpio pin low
// this turns off 3.3v power to mcu
// this never returns;
void powerDown() {
  pwron_clr;
  while(true);
}

u8 clickCount = 0;

void clickTimeout(void) {
  if(clickCount == 1) powerDown();  
  else if(clickCount >= 2) {
    if(nightLightMode) {
        nightLightMode = true;
        setEepromByte(eeprom_mode_adr, nightLightMode);
        flash(nightLightMode);
    }
    else {
        nightLightMode = false;
        setEepromByte(eeprom_mode_adr, nightLightMode);
        flash(nightLightMode);
    }
  }
  clickCount = 0;
}

void adjBrightness(bool cw) {
  if( cw && brightness < MAX_BRIGHTNESS) {
    brightness++;
    setEepromByte(eeprom_brightness_adr, brightness);
  }
  if(!cw && brightness > 0) {
    brightness--;
    setEepromByte(eeprom_brightness_adr, brightness);
  }
}

void adjNightLightThreshold(bool cw) {
  // turning up threshold makes led turn on
  if( cw && nightlightThresh < MAX_THRESHOLD) 
      nightlightThresh += THRESH_INC;
  if(!cw && nightlightThresh > MIN_THRESHOLD) 
      nightlightThresh -= THRESH_INC;
}

u16 lastClickTime = 0;

volatile u16 buttonPressCount = 0; // debug

void buttonPress(void) {
  lastClickTime = millis();
  clickCount++;

  buttonPressCount++; // debug
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
    if(button_lvl) {
      // turning knob while knob pressed
      // sets nightlight light threshold
      adjNightLightThreshold(encb_lvl);
    }
    else {
      // turning knob while not pressed
      adjBrightness(encb_lvl);
    }
    lastEncaActivity = now;
    encaWaitDebounce = true;
  }
}

void initInput(void) {
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
}

#define CLICK_DELAY     300  // 300 ms  timeout for counting clicks
#define SETTING_DELAY 30000  //  30 sec timeout while setting

// called every timer interrupt (64 usecs) from led.c
// runs at highest interrupt priority
void inputLoop(void) {
  bool justPoweredOn = true;
  u16 now = millis();

  if(justPoweredOn) {
    flash(nightLightMode);
    justPoweredOn = false;
  }

  // click delay timeout starts on button release
  if(button_lvl) lastBtnActivity = now;

  if(clickCount > 0 && ((now - lastClickTime) > CLICK_DELAY)) 
    clickTimeout();
}

