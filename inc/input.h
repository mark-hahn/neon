#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// dayBrightness (1..9) 
// the max holds with battery down to 3.35v
#define MAX_BRIGHTNESS      11  // 250 ma
#define DEFAULT_BRIGHTNESS  11  // 250 ma
#define MIN_BRIGHTNESS       1  //  night: 0.4ma, day: 15ma

#define NIGHTLIGHT_THRESHOLD_INC   10  
#define MAX_NIGHTLIGHT_THRESHOLD  600
#define DEF_NIGHTLIGHT_THRESHOLD   50  
#define MIN_NIGHTLIGHT_THRESHOLD   20
#define THRESHOLD_HYSTERISIS       10
#define DEFAULT_THRESHOLD         100

#define BUTTON_DOWN   button_lvl

// these are stored in eeprom
extern bool nightMode;
extern u16  nightThresh;
extern u8   nightBrightness;  // 1-9,  9 => 12 ma
extern u8   dayBrightness;    // 1-9,  9 => 250 ma

// active flag lasts 6 secs after knob used
// this keeps led lit while adjusting night threshold
#define INPUT_ACTIVE_DUR_MS 3000
extern bool inputActive;

// irq6 interrupt every button pin rising edge (port D)
@far @interrupt void buttonIntHandler(void);   // irq6

// irq5 interrupt on either encoder pin rising edge (port C)
@svlreg @far @interrupt void encoderIntHandler(void);  // irq5

void powerDown(void);

void initInput(void);
// called every timer interrupt (500 usecs) from led.c
void inputLoop(void);
 
#endif




