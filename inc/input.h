#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// brightness (-1..6)  is 2^^brightness ma,  1/2..64 ma
#define MIN_BRIGHTNESS -1 
#define MAX_BRIGHTNESS  6 

// speed (0..10) is 2^^(speed-4) secs,  1/16..64 secs per action
#define MAX_SPEED      10 

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void buttonIntHandler();   // irq6

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void encoderIntHandler();  // irq5

void initInput(void);

// called every timer interrupt (64 usecs) from led.c
void inputLoop(void);
 
#endif




