#include "clock.h"
#include "gpio.h"
#include "main.h"

volatile u16 msCounter;

// time in ms like arduino, but wraps every 65 seconds
u16 millis() {
  u16 ms;
  ints_off;
  ms = msCounter;
  ints_on;
  return ms;
}

// fast counter, resets every 1ms (8 counts of 128us each)
volatile u8 clkCnt = 0;

// interrupt routine, happens every 128 usecs
@far @interrupt void tim4IntHandler() {
  TIM4->SR1 = 0; // int flag off
  if(++clkCnt == INTS_PER_MS) {
    clkCnt = 0;
    msCounter++;
  }
}

void initClock(void) {
  // set IRQ23 software interrupt priority to 1 (lowest)
  ITC->ISPR6 = (ITC->ISPR6 & ~0xc0) | 0x40; // I1I0 is 0b01 

  TIM4->PSCR = TIM4_PRESCALE; // 2 MHz counter rate
  TIM4->ARR  = 255;           // auto-reload, 256 counts, 128 usecs per overflow/int
  TIM4->CR1 = 
      // TIM4_CR1_ARPE | // ((uint8_t)0x80) /*!< Auto-Reload Preload Enable mask. */
      // TIM4_CR1_OPM  | // ((uint8_t)0x08) /*!< One Pulse Mode mask. */
      // TIM4_CR1_URS  | // ((uint8_t)0x04) /*!< Update Request Source mask. */
      // TIM4_CR1_UDIS | // ((uint8_t)0x02) /*!< Update DIsable mask. */
      TIM4_CR1_CEN;      // ((uint8_t)0x01) /*!< Counter Enable mask. */
  TIM4->IER = TIM4_IER_UIE;     // enable timer interrupts
}

void clockLoop() {

}

