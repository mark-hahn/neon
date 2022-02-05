#ifndef _LED_
#define _LED_

#include "stm8s.h"
#include "main.h"

// pwm is pin a3 -- TIM2_3

#define MAX_PWM       1024  // TODO -- measure for 50 ma
#define MAX_CURRENT 0xffff  // TODO -- measure for 50 ma

// battery divided by this before multipling battery sense adc
#define BAT_FACTOR            2
#define NOT_DIMMING_FACTOR 1024

extern i8  brightness;
extern u16 dimFactor;

// returns elapsed ms, rolls over every 4 secs (64 usecs * 65536)
u16 millis(void);

@far @interrupt void tim2IntHandler();

void initLed(void);

#endif  // _LED_

