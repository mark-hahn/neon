#ifndef _LED_
#define _LED_

#include "stm8s.h"
#include "main.h"

// directly affects adc tgt and led current
// 8: brightness of 1 => 1.5 ma & 14 => 159 ma
#define LED_ADC_TGT_FACTOR 8
#define MAX_LED_ADC_TGT  700  // max seen should be 600
#define MAX_PWM         1024  // full battery voltage

// returns elapsed ms, rolls over every 4 secs (64 usecs * 65536)
u16 millis(void);

@far @interrupt void tim2IntHandler();

void setLedAdcTgt(void);
void initLed(void);

#endif  // _LED_

