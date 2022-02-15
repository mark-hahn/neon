#ifndef _LED_
#define _LED_

#include "stm8s.h"
#include "main.h"

// pwm is pin a3 -- TIM2_3

// brightness (0..14) is index for brightness of 0..128 ma
// 0b0, 0b1, 0b10, 0b11, 0b100, 0b110,..., 0b10000000 ma
#define MAX_BRIGHTNESS     14  // 128 ma
#define DEFAULT_BRIGHTNESS 10  //  32 ma

#define THRESH_INC      10  // threshold change on knob turn
#define MAX_THRESHOLD  512  // max nightlight adc value
#define MIN_THRESHOLD  512  // min nightlight adc value
#define THRESHOLD_HIST  50  // nightlight hysterises

#define MAX_PWM       1024  // TODO -- measure for 50 ma
#define MAX_CURRENT 0xffff  // TODO -- measure for 50 ma

// battery divided by this before multipling battery sense adc
#define BAT_FACTOR            2

// simple indexes
extern u8  brightness; 

// returns elapsed ms, rolls over every 4 secs (64 usecs * 65536)
u16 millis(void);

@far @interrupt void tim2IntHandler();

void initLed(void);

#endif  // _LED_

