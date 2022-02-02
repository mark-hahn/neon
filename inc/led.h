#ifndef _LED_
#define _LED_

#include "stm8s.h"
#include "main.h"

// pwm is pin a3 -- TIM2_3

@far @interrupt void tim2IntHandler();

// returns elapsed ms, rolls over every 4 secs (64 usecs * 65536)
u16 millis(void);

// flashes led count times
void flash(u8 count);

void initLed(void);

#endif  // _LED_

