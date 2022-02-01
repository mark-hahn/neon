#include "stm8s.h"
#include "main.h"
#include "gpio.h"

// interrupt routine, happens every button/encoder pin change
@far @interrupt void inputIntHandler() {
  TIM4->SR1 = 0; // int flag off
  if(++clkCnt == INTS_PER_MS) {
    clkCnt = 0;
    msCounter++;
  }
}

void initInput(void) {
  // set encoder (IRQ5) software interrupt priority to 1 (lowest)
  ITC->ISPR2 = (ITC->ISPR2 & ~0x0c) | 0x04; // I1I0 is 0b01 

  // set button (IRQ6) software interrupt priority to 1 (lowest)
  ITC->ISPR2 = (ITC->ISPR2 & ~0x30) | 0x10; // I1I0 is 0b01 

}