
#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "led.h"
#include "adc.h"

#define CH_MASK   0x0f  // in ADC1->CSR

#define BSENS_CH     2
#define CURSENS_CH   4
#define LGTSENS_CH   5

// assume 3.7 on startup
u16 lastBsensVal = 0x0200;
u16 vbatFactor   = 0x0200;  // need to measure

u16 setVbatFactor() {
  // TO-DO measure adc for different vbat
  // to-do read vbat adc
}

u8 curAdcChan = 2; // channel being converted

void startAdcChan(u8 chan) {
  if(chan != CURSENS_CH) curAdcChan = chan;
  ADC1->CSR  = (ADC1->CSR & ~CH_MASK) | chan;
  ADC1->CR1 |= ADC1_CR1_ADON; // start conversion
}

// wait or conversion to finish and return value
u16 waitForAdc() {
  while ((ADC1_CSR->CR1 & ADC1_CSR_EOC) == 0);
  ADC1->CSR &= ~ADC1_CSR_EOC; // turn off end-of-conversion flag
  return (ADC1->DRH << 8) | ADC1->DRL;
}

// led current sensor from adc
// pre-adjusted by battery voltage
// called from led.c
u16 getLedCurrent() {

}

lastLgtsensVal = 0x0200;

u16 getLightAdc() {

}


// ambient light sensor (photo-resistor) from adc
// pre-adjusted by battery voltage
// called from input.c
u16 getAmbientLight() {


}

void initAdc(void) {
  // set gpio pins as inputs to avoid driving pins not being converted
  bsens_in;    // C4  ADC2
  cursens_in;  // D3  ADC4
  lgtsens_in;  // D5  ADC5

  ADC1->CSR = 
    // ADC1_CSR_EOC   |  // 0x80 End of Conversion mask
    // ADC1_CSR_AWD   |  // 0x40 Analog Watch Dog Status mask
    // ADC1_CSR_EOCIE |  // 0x20 Interrupt Enable for EOC mask
    // ADC1_CSR_AWDIE |  // 0x10 Analog Watchdog interrupt enable mask
    | // ADC1_CSR_CH 0x0F Channel selection bits mask (AINx) -- set to select
    0;

  ADC1->CR1 =
    0x70 | // ADC1_CR1_SPSEL 0x70 Prescaler selection mask (0.9 MHz, 16 MHz/18)
    // ADC1_CR1_CONT  | // 0x02 Continuous conversion mask
    // ADC1_CR1_ADON  | // 0x01 A/D Converter on/off mask (set at end)
    0;

  ADC1->CR2 =
    // ADC1_CR2_EXTTRIG | 0x40 External trigger enable mask
    // ADC1_CR2_EXTSEL  | 0x30 External event selection mask
    ADC1_CR2_ALIGN      | // 0x08 Data Alignment mask (Right alignment, 9:8 in DRH, 7:0 in DRL)
    // ADC1_CR2_SCAN    | 0x02 Scan mode mask
    0;

  ADC1->CR3 = 0; // (unused, no data buffering)
    // ADC1_CR3_DBUF    ((uint8_t)0x80) /*!< Data Buffer Enable mask
    // ADC1_CR3_OVR     ((uint8_t)0x40) /*!< Overrun Status Flag mask

  startAdcChan(BSENS_CH);  // wake up adc and start first conversion
}

// called from timer int in led.c
// returns led cur sens value
u16 handleAdcInt(void) {
  u16 curSensAdcVal;
  // wait for previous BSENS_CH or LGTSENS_CH conversion
  // save adc value
  if(curAdcChan == BSENS_CH) lastBsensVal   = waitForAdc(); 
  else                       lastLgtsensVal = waitForAdc(); 

  startAdcChan(CURSENS_CH);
  curSensAdcVal = waitForAdc();

  // start next conversion to run between interrupts;
  if(curAdcChan == BSENS_CH) startAdcChan(LGTSENS_CH); 
  else                       startAdcChan(BSENS_CH); 

  return curSensAdcVal;
}
