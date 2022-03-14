// STM8S003F3 or STM8S103F3 

#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "input.h"
#include "adc.h"
#include "led.h"
#include "eeprom.h"

void main(void) {
  // turn on power pin quickly to keep power on
  pwron_set;
  pwron_out;

  // switch high-speed internal oscillator (HSI) to full-speed
  CLK->CKDIVR   = 0;

  initEeprom(); // must be first
  initInput();
  initAdc();
  initLed();

  enableInterrupts(); 

  // everything is interrupt driven
  // so from now on all code runs in int mode
  // waits in low-power wait mode between interrupts
	while(true) wfi();}
