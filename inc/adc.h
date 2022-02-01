#ifndef __ADC__
#define __ADC__

#define ADC_NULL 0xffff

// each adc channel conversion delay
#define ADC_UPDATE_MS 1 

// headlight sense resistor voltage adc values
extern u16 hdlgtAdcL;
extern u16 hdlgtAdcR;

void initAdc(void);
void adcLoop(void);
 
#endif
