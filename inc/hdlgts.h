#ifndef __HDLGTS__
#define __HDLGTS__

#include "stm8s.h"
#include "main.h"

// set alternative function register 0, afr0
// left motor:     tim1_1, tim1_2
// left headlight: tim2_1


#define HDLGT_PWM_MAX 1024  // period == 16 KHz (counter counts at 16 MHz)

// set headlight brightness by specifying led current
// current is in ma units
// set by commands from cpu over i2c
void setHdlgt(side_t side, bool flash, u16 pwm);

void initHdlgts(void);
void hdlgtsLoop(void);

#endif  // __HDLGTS__

