#ifndef __ADC__
#define __ADC__

#include "stm8s.h"
#include "main.h"

// ambient light (photo-resistor)
// meaningless units
extern u16 lightSens; 

void initAdc(void);

// called from timer int in led.c
// returns led current, no meaningful units
u16 handleAdcInt(void);
 
#endif
