#ifndef _LED_
#define _LED_

#include "stm8s.h"
#include "main.h"

#define TIM2_PRESCALE   3  // 16MHz / (2**TIM2_PRESCALE) => 2 MHz
#define INTS_PER_MS     2  // pwm freq == 2 KHz (compared to 200 Hz RC)

#define LED_PWM_MAX   1023  // timer rolls over every 500 usecs
#define LED_PWM_L TIM2->CCR3L
#define LED_PWM_H TIM2->CCR3H

// these are used to limit the integral and the pwm
#define MAX_PWM  900  // gives 300ma (for 3 led strands)
#define MIN_PWM  200  // slightly smaller than 4.2v turn-on

// PID constants, always power of 2 for calc speed
#define Ik  8  // divide   PID integral by 2**Ik
#define Pk  2  // multiply PID error    by 2**Pk

#define MIN_INTEGRAL  (((i32) MIN_PWM) << Ik)
#define MAX_INTEGRAL  (((i32) MAX_PWM) << Ik)

// range used to dim day brightness
#define MAX_LIGHT_FACTOR   900
#define MIN_LIGHT_FACTOR    20
 
// led adc target is multiplied by 6/2**18 
#define ADC_TGT_ADJ      6
#define ADC_TGT_FACTOR  18

// returns elapsed ms, rolls over every 65 secs
u16 millis(void);

// wait 10 ms for everything to stabilize
#define PWR_ON_DELAY_MS  10  
extern bool pwrOnStabilizing;

@svlreg @far @interrupt void tim2IntHandler();

void setLedAdcTgt(u16 batteryAdc);
void initLed(void);

#endif  // _LED_

