#ifndef _LED_
#define _LED_

#include "stm8s.h"
#include "main.h"

// pwm is pin a3 -- TIM2_3

#define TIM2_PRESCALE    0  // clocks at full 16 MHz
#define LED_PWM_MAX   1024  // rolls over every 64 usecs
#define INTS_PER_MS     16  // pwm freq == 16 KHz (compared to 100 Hz RC)

@far @interrupt void tim2IntHandler();

// set led brightness, raw pwm units (0 - 1023)
ledPwm(u16 pwm);
// returns elapsed ms, rolls over every 4 secs (64 usecs * 65536)
u16 millis(void);

void initLed(void);
void ledLoop(void);

#endif  // _LED_

