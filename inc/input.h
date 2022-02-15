#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// operation modes
enum {
  modeNormal,
  modeNightLight,
  modeAdjust
};

extern bool nightLightMode;
extern u16  nightlightThresh;

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void buttonIntHandler(void);   // irq6

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void encoderIntHandler(void);  // irq5

void powerDown(void);

void initInput(void);

// called every timer interrupt (64 usecs) from led.c
void inputLoop(void);
 
#endif




