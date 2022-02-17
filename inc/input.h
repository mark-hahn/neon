#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// brightness (0..16) (could probably be bigger)
#define MAX_BRIGHTNESS     14  // 231 ma
#define DEFAULT_BRIGHTNESS 12  // 115 ma

// todo -- measure this
#define MAX_NIGHTLIGHT_THRESHOLD 200
#define DEF_NIGHTLIGHT_THRESHOLD 100  

// operation modes
enum {
  modeNormal,
  modeNightLight,
  modeAdjust
};

extern bool nightLightMode;
extern u8   nightlightThresh;
extern u8   brightness;  // 0-16, 16 => 140 ma

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void buttonIntHandler(void);   // irq6

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void encoderIntHandler(void);  // irq5

void powerDown(void);

void initInput(void);

// called every timer interrupt (64 usecs) from led.c
void inputLoop(void);
 
#endif




