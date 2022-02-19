#ifndef _LED_
#define _LED_

#include "stm8s.h"
#include "main.h"

// adjust brightness here
// directly affects adc tgt and led current
// 8: brightness of 1 => 1.5 ma & 14 => 159 ma
#define LED_ADC_TGT_FACTOR 8
#define MAX_LED_ADC_TGT  128  // max seen should be 106
#define MAX_PWM         1024  // full battery voltage

// returns elapsed ms, rolls over every 4 secs (64 usecs * 65536)
u16 millis(void);

// wait 10 ms for everything to stabilize
#define PWR_ON_DELAY_MS  10  
extern bool pwrOnStabilizing;

// flash led count+1 times
void flash(u8 count);

@svlreg @far @interrupt void tim2IntHandler();

void setLedAdcTgt(u16 batteryAdc);
void initLed(void);

#endif  // _LED_

