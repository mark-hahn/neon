#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// brightness (0..12) 
// the max holds with battery down to 3.35v
#define MAX_BRIGHTNESS     12  // 159 ma
#define DEFAULT_BRIGHTNESS 10  //  80 ma

// todo -- measure this
#define NIGHTLIGHT_THRESHOLD_INC  20  
#define MAX_NIGHTLIGHT_THRESHOLD 800
#define DEF_NIGHTLIGHT_THRESHOLD 700  
#define MIN_NIGHTLIGHT_THRESHOLD 600
#define THRESHOLD_HISTERISIS      10

// range used to dim brightness (set lightFactor in led.c)
#define MAX_LIGHT_ADC   800
#define MIN_LIGHT_ADC   700
#define LIGHT_ADC_RANGE (MAX_LIGHT_ADC - MIN_LIGHT_ADC)

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




