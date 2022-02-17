#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// brightness (0..16) (could probably be bigger)
#define MAX_BRIGHTNESS     14  // 159 ma
#define DEFAULT_BRIGHTNESS 12  //  80 ma

// todo -- measure this
#define NIGHTLIGHT_THRESHOLD_INC  20  
#define MAX_NIGHTLIGHT_THRESHOLD 400
#define DEF_NIGHTLIGHT_THRESHOLD 300  
#define MIN_NIGHTLIGHT_THRESHOLD 200
#define THRESHOLD_HISTERISIS      10

extern bool nightLightMode;
extern u8   nightlightThresh;
extern u8   brightness;  // 0-14, 14 => 159 ma

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void buttonIntHandler(void);   // irq6

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void encoderIntHandler(void);  // irq5

void powerDown(void);

void initInput(void);

// called every timer interrupt (64 usecs) from led.c
void inputLoop(void);
 
#endif




