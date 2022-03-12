#ifndef _LED_
#define _LED_

#include "stm8s.h"
#include "main.h"

// these are only to avoid slow locking from extreme I values
#define MAX_DAY_PWM 820  // slightly bigger than 3v turn-on
#define MAX_PWM     900  // gives 300ma (for 3 led strands)
#define MIN_PWM     200  // slightly smaller than 4.2v turn-on

// this directly affects lock speed and stability
#define PWM_DIV       6  // divide PID integral by 2**PWM_DIV

// adjust dayBrightness here
// directly affects adc tgt and led current
// 8: dayBrightness of 1 => 1.5 ma & 14 => 159 ma
#define LED_ADC_TGT_FACTOR     8
#define MAX_DAY_LED_ADC_TGT  256  // protect led strand from > 100ma

// returns elapsed ms, rolls over every 4 secs (64 usecs * 65536)
u16 millis(void);

// wait 10 ms for everything to stabilize
#define PWR_ON_DELAY_MS  10  
extern bool pwrOnStabilizing;

@svlreg @far @interrupt void tim2IntHandler();

void setLedAdcTgt(u16 batteryAdc);
void initLed(void);

#endif  // _LED_

