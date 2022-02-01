
#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "clock.h"
#include "adc.h"

// ADC_NULL is flag indicating value not set
u16 hdlgtAdcL = ADC_NULL;
u16 hdlgtAdcR = ADC_NULL;

#define NUM_ADC_CHANNELS 3
const u8 adcAin[NUM_ADC_CHANNELS] = {adc_ain, hdlvl_ain, hdlvr_ain};

void setAdcVal(u8 idx, u16 data) {
  switch(idx) {
    case 0: setAdcSendData(data);                          break;
    case 1: setHdlgtAdcDebugDataL(data); hdlgtAdcL = data; break;
    case 2: setHdlgtAdcDebugDataR(data); hdlgtAdcR = data; break;
  }
}

// counts from 0 to NUM_ADC_CHANNELS-1
u8 adcIdx = 0;

void initAdc(void) {
  // set gpio pins as inputs to avoid driving pins not being converted
  adc_in;
  hdlvl_in;
  hdlvr_in;

  adcIdx = 0; // first channel (ain pin)

  ADC1->CSR = 
    // ADC1_CSR_EOC   |  // 0x80 End of Conversion mask
    // ADC1_CSR_AWD   |  // 0x40 Analog Watch Dog Status mask
    // ADC1_CSR_EOCIE |  // 0x20 Interrupt Enable for EOC mask
    // ADC1_CSR_AWDIE |  // 0x10 Analog Watchdog interrupt enable mask
    adcIdx            |  // ADC1_CSR_CH 0x0F Channel selection bits mask (AINx)
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

  ADC1->CR1 |= ADC1_CR1_ADON; // turn on ADC power and take over pins
  ADC1->CR1 |= ADC1_CR1_ADON; // start first conversion
}

u16 lastMsAdc = 0;

void adcLoop(void) {
  u16 ms = millis();
  // update one channel every 1ms
  if((ms - lastMsAdc) >= ADC_UPDATE_MS) {
    // this will always be true since adc conversion time < 1ms
    if(ADC1->CSR & ADC1_CSR_EOC) {
      // last conversion complete
      u16 adcLo = ADC1->DRL;  // low must be read first
      u16 adcHi = ADC1->DRH;
      setAdcVal(adcIdx, (adcHi << 8) | adcLo); 
      ADC1->CSR &= ~ADC1_CSR_EOC; // turn off end-of-conversion flag

      if(++adcIdx == NUM_ADC_CHANNELS) adcIdx = 0;
      ADC1->CSR = (ADC1->CSR & 0xf0) | adcAin[adcIdx]; // set next channel
      ADC1->CR1 |= ADC1_CR1_ADON;                      // start next conversion
    }
    lastMsAdc = ms;
  }
}
