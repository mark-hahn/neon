
#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "input.h"
#include "led.h"
#include "adc.h"

#define CH_MASK   0x0f  // in ADC1->CSR

#define BAT_ADC_CHAN     2
#define LED_ADC_CHAN     4
#define LIGHT_ADC_CHAN   5

u16 lightAdc   = 170;

void startAdc(u8 chan) {
  ADC1->CSR  = (ADC1->CSR & ~CH_MASK) | chan;
  ADC1->CR1 |= ADC1_CR1_ADON; // start conversion
}

// wait for conversion to finish and return value
u16 waitForAdc(void) {
  while ((ADC1->CSR & ADC1_CSR_EOC) == 0);
  ADC1->CSR &= ~ADC1_CSR_EOC; // turn off end-of-conversion flag
  return get16(ADC1->DR);
}

// channel being converted between ints
// light or battery
u8   curAdcChan = BAT_ADC_CHAN;
bool adcActive  = false;

void initAdc(void) {
  u8 i;

  // set gpio pins as inputs to avoid driving pins not being converted
  bsens_in;    // C4  ADC2
  cursens_in;  // D3  ADC4
  lgtsens_in;  // D5  ADC5

  ADC1->CSR = 
    // ADC1_CSR_EOC   |  // 0x80 End of Conversion mask
    // ADC1_CSR_AWD   |  // 0x40 Analog Watch Dog Status mask
    // ADC1_CSR_EOCIE |  // 0x20 Interrupt Enable for EOC mask
    // ADC1_CSR_AWDIE |  // 0x10 Analog Watchdog interrupt enable mask
    // ADC1_CSR_CH 0x0F Channel selection bits mask (AINx) -- set to select
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

  ADC1->CR1 |= ADC1_CR1_ADON; // first time only powers on
  for(i=0; 1 < 16*7; i++);    // wait for > 7 us for adc to stabilize

  // batterySens must be first, used immediately
  startAdc(LIGHT_ADC_CHAN); 
  lightAdc = waitForAdc();
  startAdc(BAT_ADC_CHAN); 
  curAdcChan = BAT_ADC_CHAN;
  adcActive = true;
}

// this protects the battery from under voltage
// note that the adc reading goes up as voltage goes down
// 3v => 0.7 / (3/1024) => 240
#define BAT_UNDER_VOLTAGE_THRES 239

// alternating get light or battery reading
// every 100 ms
#define ADC_INTERVAL_MS 100
// called from timer int in led.c
// returns led current in adc count
// sort of like a main loop
u16 handleAdcInt(void) {
  static u16 batteryAdc  = 0;
  static u16 lastAdcTime = 0;

  u16 ledAdc;
  u16 now = millis();
  
  // wait for previous battery or light conversion
  if(adcActive) {
    if(curAdcChan == BAT_ADC_CHAN) {
      batteryAdc = waitForAdc(); 
      // battery under-voltage protection
      if(batteryAdc > BAT_UNDER_VOLTAGE_THRES) powerDown();
    }
    else
      lightAdc = waitForAdc(); 
    lastAdcTime = now;
    adcActive   = false;
  }

  startAdc(LED_ADC_CHAN);
  ledAdc = waitForAdc();
  setLedAdcTgt(batteryAdc);

  // start next conversion to run between interrupts;
  // this happens every 100 ms
  if((now - lastAdcTime) > ADC_INTERVAL_MS) {
    if(curAdcChan == BAT_ADC_CHAN) {
      startAdc(LIGHT_ADC_CHAN); 
      curAdcChan = LIGHT_ADC_CHAN;
    }
    else {
      startAdc(BAT_ADC_CHAN); 
      curAdcChan = BAT_ADC_CHAN;
    }
    adcActive = true;
  }
  return ledAdc;
}
