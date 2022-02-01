// this code assumes STM8S003F3 or STM8S103F3 
// set these option bits ...
//   AFR0 for timers

#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "hdlgts.h"
#include "motors.h"
#include "adc.h"
#include "output.h"
#include "i2c.h"
#include "command.h"
#include "clock.h"

void main(void) {
//  while(true);  // disable mcu
	
  // switch high-speed internal oscillator (HSI) to full-speed
  CLK->CKDIVR   = 0;

  initAdc();
  initHdlgts();
  initMotors();
  initOutput();
  initClock();
  initI2c();
  initCommand();

  ints_on; // enable clock and i2c interrupts

  // quick power-on beep
  // beep(250);

  // foreground event loop
	while (true) { 
    adcLoop();
    hdlgtsLoop();    
    motorsLoop();    
    outputLoop();
    clockLoop();    
    i2cLoop();    
    commandLoop();
  }
}

#define DBG_BUF_SIZE 256
u8 @near dbgBuf[DBG_BUF_SIZE];
u16 dbgBufIdx = 0;
void trace(u8 byte) {
  dbgBuf[dbgBufIdx++] = byte;
  if(dbgBufIdx == DBG_BUF_SIZE) dbgBufIdx = 0;
}
