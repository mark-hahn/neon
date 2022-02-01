#ifndef __ADC__
#define __ADC__

// ambient light sensor (photo-resistor) from adc
// pre-adjusted by battery voltage
// called from input.c
u16 getAmbientLight();

// led current sensor from adc
// pre-adjusted by battery voltage
// called from led.c
u16 getLedCurrent();

void initAdc(void);
void adcLoop(void);
 
#endif
