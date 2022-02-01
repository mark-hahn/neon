#ifndef __ADC__
#define __ADC__

#include "main.h"

// ambient light sensor (photo-resistor) from adc
// pre-adjusted by battery voltage
// called from input.c
u16 getAmbientLight();

// led current sensor from adc
// pre-adjusted by battery voltage
// called from led.c
u16 getLedCurrent();

void initAdc(void);
u16 handleAdcInt(void);
 
#endif
