#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

// ambient light sensor (photo-resistor) from adc
// pre-adjusted by battery voltage
// called from input.c
u16 getAmbientLight();

// led current sensor from adc
// pre-adjusted by battery voltage
// called from led.c
u16 getLedCurrent();

@far @interrupt void inputIntHandler();

void initInput(void);
void inputLoop(void);
 
#endif




