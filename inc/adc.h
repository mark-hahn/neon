#ifndef __ADC__
#define __ADC__

#include "stm8s.h"
#include "main.h"

// period alternating get light or battery reading
#define ADC_INTERVAL_MS 10

// vcc powers down when battery adc higher than this
// this protects the battery from under voltage
// note that the adc reading goes up as voltage goes down
#define BAT_UNDER_VOLTAGE_THRES 165

extern u16 lightAdc; 

void initAdc(void);

// called from timer int in led.c
// returns led current adc
u16 handleAdc(void);
 
#endif
