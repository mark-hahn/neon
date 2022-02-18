#ifndef __ADC__
#define __ADC__

#include "stm8s.h"
#include "main.h"

// ambient light (photo-resistor)
extern u16 lightAdc; 

void initAdc(void);

// called from timer int in led.c
// returns led current adc
u16 handleAdcInt(void);
 
#endif
