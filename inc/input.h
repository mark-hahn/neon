#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// -- what about nightBrightness?  TODO
// dayBrightness (0..14) 
// the max holds with battery down to 3.35v
#define MAX_BRIGHTNESS     14  // 159 ma
#define DEFAULT_BRIGHTNESS 10  //  80 ma
#define MIN_BRIGHTNESS      1  //   1 ma

// todo -- measure this
#define NIGHTLIGHT_THRESHOLD_INC  20  
#define MAX_NIGHTLIGHT_THRESHOLD 100
#define DEF_NIGHTLIGHT_THRESHOLD  40  
#define MIN_NIGHTLIGHT_THRESHOLD  10
#define THRESHOLD_HISTERISIS      10

// -- what about nightBrightness?  TODO
// range used to dim dayBrightness
// sets lightFactor in led.c
#define MAX_LIGHT_ADC   160
#define MIN_LIGHT_ADC    10

extern bool nightMode;
extern u8   nightlightThresh;
extern u8   nightBrightness;  // 0-14, 14 => 15 ma
extern u8   dayBrightness;    // 0-14, 14 => 300 ma

// interrupts every button pin rising edge (port D)
@far @interrupt void buttonIntHandler(void);   // irq6

// irq5 interrupt, either encoder pin rising edge (port C)
@svlreg @far @interrupt void encoderIntHandler(void);  // irq5

void powerDown(void);

void initInput(void);

// called every timer interrupt (64 usecs) from led.c
void inputLoop(void);
 
#endif




