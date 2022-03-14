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

// global ambient light adc value
// ranges from 50 to 950
u16 lightAdc = 600; 

void startAdc(u8 chan) {
  ADC1->CSR  = (ADC1->CSR & ~CH_MASK) | chan;
  ADC1->CR1 |= ADC1_CR1_ADON; // start conversion
}

// wait for conversion to finish and return value
u16 waitForAdc(void) {
  u8 loByte, hiByte;

  // wait for conversion to finish
  while ((ADC1->CSR & ADC1_CSR_EOC) == 0);
  ADC1->CSR &= ~ADC1_CSR_EOC; // turn off end-of-conversion flag

  // loByte must be read first because of byte alignment
  loByte = ADC1->DRL;
  hiByte = ADC1->DRH;
  return (((u16) hiByte << 8) | loByte);
}

// light or battery are converted between ints
u8   curAdcChan = BAT_ADC_CHAN;
bool adcActive  = false;

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

  // true between handleAdc calls
  // when adc started for light or battery
  adcActive  = false;
}

#define ADC_HIST_LEN 32 // max 6 bits (64)

  u16 batteryAdc = 150; // debug -- belongs in handleAdc

// called from timer int in led.c every 128us
// returns led current in adc count
// sort of like a main loop
u16 handleAdc(void) {

  // filter light and battery over 32 samples
  static u16 batAdcHist[ADC_HIST_LEN] = {BAT_UNDER_VOLTAGE_THRES};  
  static u8  batAdcHistIdx = 0;
  static u16 lgtAdcHist[ADC_HIST_LEN] = {BAT_UNDER_VOLTAGE_THRES};  
  static u8  lgtAdcHistIdx = 0;

  static bool waitingToStartBatAdc = true;
  static u16 lastAdcTime = 0;
  u16 ledAdc     = 0xfff;   // keep pwm low at beginning
  u16 now = millis();
  
  // battery or light conversion happens every 10 ms
  // started at end of this function and finishes here
  if(adcActive) {
    u16 adc = waitForAdc(); 
    if(curAdcChan == BAT_ADC_CHAN) {
      u8  idx;
      u16 sum = 0;
      batAdcHist[batAdcHistIdx++] = adc;
      if(batAdcHistIdx == ADC_HIST_LEN) batAdcHistIdx = 0;
      for(idx=0; idx < ADC_HIST_LEN; idx++) sum += batAdcHist[idx];
      batteryAdc = (sum/ADC_HIST_LEN);
			
      // battery under-voltage protection
      if(batteryAdc > BAT_UNDER_VOLTAGE_THRES) powerDown();
			
      // set led adc target, uses brightness vars and battery voltage
      setLedAdcTgt(batteryAdc);
    }
    else {
      u8  idx;
      u16 sum = 0;
      lgtAdcHist[lgtAdcHistIdx++] = adc;
      if(lgtAdcHistIdx == ADC_HIST_LEN) lgtAdcHistIdx = 0;
      for(idx=0; idx < ADC_HIST_LEN; idx++) sum += lgtAdcHist[idx];
      lightAdc = (sum/ADC_HIST_LEN);
    }

    lastAdcTime = now;
    adcActive   = false;
  }

  if(!waitingToStartBatAdc) {
    startAdc(LED_ADC_CHAN);
    // returned below
    ledAdc = waitForAdc();
  }

  // start next conversion to run between interrupts;
  // this happens every 500 ms
  if((now - lastAdcTime) > ADC_INTERVAL_MS) {
    if(!waitingToStartBatAdc && 
        curAdcChan == BAT_ADC_CHAN) {
      startAdc(LIGHT_ADC_CHAN); 
      curAdcChan = LIGHT_ADC_CHAN;
    }
    else {
      startAdc(BAT_ADC_CHAN); 
      curAdcChan = BAT_ADC_CHAN;
      waitingToStartBatAdc = false;
    }
    adcActive = true;
  }
  return ledAdc;
}
