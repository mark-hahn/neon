#ifndef __OUTPUT__
#define __OUTPUT__

#include "stm8s.h"

// beep this long
void beep(u16 durationMs);

// blink false:  light flashes on/off with durationMs period, 50% duty cycle
// blink true:   light blinks off for 50ms once every durationMs
// durationMs 0: light constantly on
void pwrLgt(bool blink, u16 durationMs); 

void initOutput(void);
void outputLoop(void);
 
#endif
