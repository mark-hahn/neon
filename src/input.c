#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "eeprom.h"
#include "input.h"
#include "led.h"

// global
bool nightLightMode   = false;
u8   nightlightThresh = DEF_NIGHTLIGHT_THRESHOLD; 
u8   brightness       = DEFAULT_BRIGHTNESS;

// set pwron gpio pin low
// this turns off 3.3v power to mcu
// this never returns;
void powerDown() {
//  pwron_clr;
//  while(true;
}

u8 clickCount = 0;

void clickTimeout(void) {
  if(clickCount == 1) 
    powerDown();  
  else if(clickCount >= 2) {
    nightLightMode = !nightLightMode;
    flash(nightLightMode);
    setEepromByte(eeprom_mode_adr, nightLightMode);
  }
  clickCount = 0;
}

void adjBrightness(bool cw) {
  if( cw && brightness < MAX_BRIGHTNESS)
    brightness++;
  if(!cw && brightness > 0)
    brightness--;
  setEepromByte(eeprom_brightness_adr, brightness);
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
@far @interrupt void buttonIntHandler() {
//  static bool btnWaitDebounce = false;
  u16 now = millis();
	
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

volatile u16 encIntCount    = 0; // debug
volatile u16 cwCount        = 0; // debug
volatile u16 ccwCount       = 0; // debug

// irq5 interrupt, either encoder pin rising edge (port C)
@far @interrupt void encoderIntHandler() {
  static bool lastencAHigh = true;
  u16 now = millis();
	
  bool encAHigh       = (enca_lvl != 0);
	bool encARisingEdge = (encAHigh && !lastencAHigh);
	lastencAHigh = encAHigh;
	
	encIntCount++;
	
	if(!encARisingEdge) return;  

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

#define CLICK_DELAY     300  // 300 ms timeout for counting clicks

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

