#ifndef __ADC__
#define __ADC__

#include "stm8s.h"
#include "main.h"

// period alternating get light or battery reading
#define ADC_INTERVAL_MS 500

// this protects the battery from under voltage
// note that the adc reading goes up as voltage goes down
// measured 3v drop-out, with 160 threshold adc
// diode drop is (3/1024)*160 => 0.47v
// 47k is too low for diode pull-up
#define BAT_UNDER_VOLTAGE_THRES 160

extern u16 lightAdc; 

void initAdc(void);

// called from timer int in led.c
// returns led current adc
u16 handleAdcInt(void);
 
#endif
