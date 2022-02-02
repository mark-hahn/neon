#ifndef __ADC__
#define __ADC__

#include "stm8s.h"
#include "main.h"

// ambient light (photo-resistor)
// meaningless units
// called from input.c
u16 getAmbientLight();

void initAdc(void);
u16 handleAdcInt(void);
 
#endif
