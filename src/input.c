#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "eeprom.h"
#include "input.h"
#include "led.h"

// vars stored in eeprom
// these initial values are only used in eeprom init
bool nightMode        = false;
u8   nightlightThresh = DEF_NIGHTLIGHT_THRESHOLD; 
u8   dayBrightness    = DEFAULT_BRIGHTNESS;
u8   nightBrightness  = DEFAULT_BRIGHTNESS;

// set pwron gpio pin low
// this turns off 3.3v power to mcu
// this never returns;
void powerDown() {  
  pwron_clr;
  while(true);
}

u8 clickCount = 0;

void clickTimeout(void) {
  if(clickCount == 1) 
    powerDown();  
  else if(clickCount >= 2) {
    nightMode = !nightMode;
    setEepromByte(eeprom_night_mode_adr, nightMode);
  }
  clickCount = 0;
}

void adjBrightness(bool cw) {
  if(nightMode) {
    if( cw && nightBrightness < MAX_BRIGHTNESS) nightBrightness++;
    if(!cw && nightBrightness > MIN_BRIGHTNESS) nightBrightness--;
    setEepromByte(eeprom_night_brightness_adr,  nightBrightness);
  }
  else {
    if( cw && dayBrightness < MAX_BRIGHTNESS) dayBrightness++;
    if(!cw && dayBrightness > MIN_BRIGHTNESS) dayBrightness--;
    setEepromByte(eeprom_day_brightness_adr,  dayBrightness);
  }
}

void adjNightLightThreshold(bool cw) {
  // turning up threshold makes led turn on
  if( cw && nightlightThresh < MAX_NIGHTLIGHT_THRESHOLD)
      nightlightThresh += NIGHTLIGHT_THRESHOLD_INC;
  if(!cw && nightlightThresh > MIN_NIGHTLIGHT_THRESHOLD) 
      nightlightThresh -= NIGHTLIGHT_THRESHOLD_INC;
  setEepromByte(eeprom_threshold_adr, nightlightThresh);
}

u16 lastPressTime = 0;

void buttonPress(void) {
  lastPressTime = millis();
  clickCount++;
}

u16 lastEncActivity = 0;

#define DEBOUNCE_DELAY_MS 1  // ignore interrupts 1-2 ms after first
u16 lastBtnPressMs = 0;

bool btnWaitDebounce = false;

bool inputActive = false;
u16  lastInputActivity = 0;

// irq6 interrupt, button pin rising edge (port D)
@svlreg @far @interrupt void buttonIntHandler() {
  u16 now = millis();
  lastInputActivity = now;
	inputActive = true;

  if(pwrOnStabilizing) return;

	if(btnWaitDebounce && 
		   ((now - lastBtnPressMs) > DEBOUNCE_DELAY_MS))
	  btnWaitDebounce = false;

  if(!btnWaitDebounce) {
    // level should always be high since only interrupts on rising edge
    // buttonPress called instantly unless during bounce delay
    buttonPress();
    // ignore ints for 1 to 2 ms
    lastBtnPressMs  = now;
    btnWaitDebounce = true;
  }
}

bool encAWaitDebounce = false;

// irq5 interrupt, either encoder pin rising edge (port C)
@far @interrupt void encoderIntHandler() {
  static bool lastencAHigh = true;
  u16 now = millis();

  bool encAHigh       = (enca_lvl != 0);
	bool encARisingEdge = (encAHigh && !lastencAHigh);
	lastencAHigh = encAHigh;
  if(!encARisingEdge) return;

  lastInputActivity = now;
  inputActive = true;

  if(pwrOnStabilizing) return;

	if(encAWaitDebounce && 
	    ((now - lastEncActivity) > DEBOUNCE_DELAY_MS))
	  encAWaitDebounce = false;

  if(!encAWaitDebounce) {
		bool cw = encb_lvl;

    if(BUTTON_DOWN) {
		  // turning knob while knob pressed
      // set nightlight light threshold
      adjNightLightThreshold(cw);
    }
    else {
      // turning knob while not pressed
      adjBrightness(cw);
    }
    lastEncActivity = now;
    encAWaitDebounce = true;
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

// 300 ms timeout for counting clicks
#define CLICK_DELAY  300  

// called every timer interrupt (500 usecs) from led.c
void inputLoop(void) {
  u16 now = millis();

  if(inputActive && ((now-lastInputActivity) > INPUT_ACTIVE_DUR_MS))
    inputActive = false;

  // click delay timeout starts on button release
  if(BUTTON_DOWN) lastBtnPressMs = now;

  if(btnWaitDebounce && 
	    ((now - lastBtnPressMs) > DEBOUNCE_DELAY_MS))
    btnWaitDebounce = false;

  if(encAWaitDebounce && 
	    ((now - lastEncActivity) > DEBOUNCE_DELAY_MS))
    encAWaitDebounce = false;

  if(clickCount > 0 && ((now - lastPressTime) > CLICK_DELAY)) 
    clickTimeout();
}

