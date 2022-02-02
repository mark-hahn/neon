#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void buttonIntHandler();   // irq6

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void encoderIntHandler();  // irq5

void initInput(void);

// called every timer interrupt (64 usecs) from led.c
void inputLoop(void);
 
#endif




