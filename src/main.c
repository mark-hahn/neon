// this code assumes STM8S003F3 or STM8S103F3 
// set these option bits ...
//   AFR0 for timers

#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "input.h"
#include "led.h"

void main(void) {
//  while(true);  // disable mcu
	
  // switch high-speed internal oscillator (HSI) to full-speed
  CLK->CKDIVR   = 0;

  initAdc();
  initInput();
  initLed();

  ints_on; // enable all interrupts

  // everything is interrupt driven
  // so from now on all code runs in int mode
  // in low-power wait mode until interrupt
  while(true) wait();
}

#define DBG_BUF_SIZE 256
u8 @near dbgBuf[DBG_BUF_SIZE];
u16 dbgBufIdx = 0;
void trace(u8 byte) {
  dbgBuf[dbgBufIdx++] = byte;
  if(dbgBufIdx == DBG_BUF_SIZE) dbgBufIdx = 0;
}
