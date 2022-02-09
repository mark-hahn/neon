// STM8S003F3 or STM8S103F3 

#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "input.h"
#include "adc.h"
#include "led.h"
#include "animation.h"
#include "eeprom.h"

void main(void) {
  // turn on power pin quickly to keep power on
  pwron_set;
  pwron_out;

//  while(true);  // disable mcu -- debug
	
  // switch high-speed internal oscillator (HSI) to full-speed
  CLK->CKDIVR   = 0;

  initInput();
  initAdc();
  initLed();
  initEeprom();

  ints_on; // enable all interrupts

  // everything is interrupt driven
  // so from now on all code runs in int mode
  // waits in low-power wait mode until interrupt
  // while(true) wait();
  while(true)  ;  // debug, cpu always running
}

#define DBG_BUF_SIZE 256
u8 @near dbgBuf[DBG_BUF_SIZE];
u16 dbgBufIdx = 0;
void trace(u8 byte) {
  dbgBuf[dbgBufIdx++] = byte;
  if(dbgBufIdx == DBG_BUF_SIZE) dbgBufIdx = 0;
}
