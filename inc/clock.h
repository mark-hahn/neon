#ifndef __CLOCK__
#define __CLOCK__

#include "stm8s.h"

#define TIM4_PRESCALE 3  // clocks at 2 MHz, interrupts every 128 usecs
#define INTS_PER_MS   8

u16 millis(void);

@far @interrupt void tim4IntHandler();

void initClock(void);
void clockLoop(void);

#endif
