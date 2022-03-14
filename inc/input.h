#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// dayBrightness (0..14) 
// the max holds with battery down to 3.35v
#define MAX_BRIGHTNESS     11  // 300 ma
#define DEFAULT_BRIGHTNESS  8  // 200 ma
#define MIN_BRIGHTNESS      1  //  night: 0.4ma, day: 15ma

// todo -- measure this
#define NIGHTLIGHT_THRESHOLD_INC  20  
#define MAX_NIGHTLIGHT_THRESHOLD 100
#define DEF_NIGHTLIGHT_THRESHOLD  40  
#define MIN_NIGHTLIGHT_THRESHOLD  10
#define THRESHOLD_HYSTERISIS      10

extern bool nightMode;
extern u8   nightlightThresh;
extern u8   nightBrightness;  // 0-14, 14 => 15 ma
extern u8   dayBrightness;    // 0-14, 14 => 300 ma

// active flag lasts 6 secs
#define INPUT_ACTIVE_DUR_MS 6000  // active flag lasts 6 secs
extern bool inputActive;

// interrupts every button pin rising edge (port D)
@far @interrupt void buttonIntHandler(void);   // irq6

// irq5 interrupt, either encoder pin rising edge (port C)
@svlreg @far @interrupt void encoderIntHandler(void);  // irq5

void powerDown(void);

void initInput(void);

// called every timer interrupt (128 usecs) from led.c
void inputLoop(void);
 
#endif




