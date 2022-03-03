#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "eeprom.h"
#include "input.h"
#include "led.h"

// global
// these values are only used in eeprom init
bool nightMode        = false;
u8   nightlightThresh = DEF_NIGHTLIGHT_THRESHOLD; 
u8   dayBrightness    = DEFAULT_BRIGHTNESS;
u8   nightBrightness  = DEFAULT_BRIGHTNESS;

// set pwron gpio pin low
// this turns off 3.3v power to mcu
// this never returns;
void powerDown() {  
  pwron_clr;
//  while(true; // debug
}

u8 clickCount = 0;

void clickTimeout(void) {
  if(clickCount == 1) 
    powerDown();  
  else if(clickCount >= 2) {
    nightMode = !nightMode;
    flash(nightMode);
    setEepromByte(eeprom_nite_mode_adr, nightMode);
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

#define DEBOUNCE_DELAY_MS 1  // ignore interrupts 1-2 ms after first
u16 lastBtnPressMs = 0;

bool btnWaitDebounce = false;

// irq6 interrupt, button pin rising edge (port D)
@svlreg @far @interrupt void buttonIntHandler() {
  u16 now = millis();
	
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
u16  lastEncAActivity = 0;

volatile u8 encIntCount    = 0; // debug
volatile u8 cwCount        = 0; // debug
volatile u8 ccwCount       = 0; // debug

bool lastencAHigh = true;

// irq5 interrupt, either encoder pin rising edge (port C)
@far @interrupt void encoderIntHandler() {
  u16 now = millis();

  bool encAHigh       = (enca_lvl != 0);
	bool encARisingEdge = (encAHigh && !lastencAHigh);
	encIntCount++; // debug
	lastencAHigh = encAHigh;
	
  if(pwrOnStabilizing) return;

	if(encAWaitDebounce && 
	    ((now - lastEncAActivity) > DEBOUNCE_DELAY_MS))
	  encAWaitDebounce = false;

  if(!encAWaitDebounce) {
		bool cw = encb_lvl;
		
		// debug
    if(cw) cwCount++;
    else   ccwCount++;

    if(button_lvl) {
		  // turning knob while knob pressed
      // set nightlight light threshold
      adjNightLightThreshold(cw);
    }
    else {
      // turning knob while not pressed
      adjBrightness(cw);
    }
    lastEncAActivity = now;
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

// called every timer interrupt (64 usecs) from led.c
// runs at highest interrupt priority
void inputLoop(void) {
  static bool firstLoop = true;
  u16 now = millis();

  if(firstLoop) {
    flash(nightMode);
    firstLoop = false;
  }
  // click delay timeout starts on button release
  if(button_lvl) lastBtnPressMs = now;

  if(btnWaitDebounce && 
	    ((now - lastBtnPressMs) > DEBOUNCE_DELAY_MS))
    btnWaitDebounce = false;

  if(encAWaitDebounce && 
	    ((now - lastEncAActivity) > DEBOUNCE_DELAY_MS))
    encAWaitDebounce = false;

  if(clickCount > 0 && ((now - lastPressTime) > CLICK_DELAY)) 
    clickTimeout();
}

