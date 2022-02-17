#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "input.h"
#include "led.h"

#define TIM2_PRESCALE    0  // clocks at full 16 MHz
#define INTS_PER_MS     16  // pwm freq == 16 KHz (compared to 100 Hz RC)

#define LED_PWM_MAX   1024  // timer rolls over every 64 usecs
#define LED_PWM_L TIM2->CCR2L
#define LED_PWM_H TIM2->CCR2H

volatile u16 msCounter;

// returns elapsed ms, rolls over every 65 secs
// called by input.c which runs at low priority
u16 millis(void) {
  u16 ms;
  // running at input low priority
  ms = msCounter;
  return ms;
}

// 2**(idx/2)  (except for idx == 0 && idx == 32)
const u16 expTable[33] = {0x0000,
  0x0001, 0x0002, 0x0003, 0x0004, 0x0006, 0x0008, 0x000b, 0x0010, 
  0x0017, 0x0020, 0x002d, 0x0040, 0x005b, 0x0080, 0x00b5, 0x0100, 
  0x016a, 0x0200, 0x02d4, 0x0400, 0x05a8, 0x0800, 0x0b50, 0x1000, 
  0x16a1, 0x2000, 0x2d41, 0x4000, 0x5a82, 0x8000, 0xb505, 0xffff};

u16 ledAdcTgt = 0;

// set desired led adc value based on brightness
void setLedAdcTgt(void) {
  static bool offDueToLight = false;
  if(nightLightMode) {
    if(offDueToLight && 
        lightAdc > (nightlightThresh + THRESHOLD_HIST)){
      offDueToLight = false; 
    }
    if(!offDueToLight && 
         lightAdc < (nightlightThresh - THRESHOLD_HIST))
      offDueToLight = true;
  }
  else
    // not nightLightMode
    offDueToLight = false;

  if(offDueToLight) {
    ledAdcTgt = 0;
    return;
  }
  // offDueToLight is off

  // -- calc target adc from brightness and battery voltage

  // for batv of 4.2v, each adc count is 4.2/1024 == 4.1mv
  // adc reading of diode is 0.7v / 4.1mv => 170 counts
  // expTable[1] == 1, expTable[12] == 64
  //  1 * 170 * (1/128) => adc count 1 
  //    1 * 0.0041 => 41mv, 0.0041 / 2.2 => 1.86 ma
  // 64 * 170 * (1/128) => adc count 85
  //   85 * 0.0041 => 0.35v,  0.35 / 2.2 => 159 ma

  // for batv of 3.35v, each adc count is 3.35/1024 == 3.3mv
  // adc reading of diode is 0.7v / 0.0033 => 212 counts
  // expTable[1] == 1, expTable[12] == 64
  //  1 * 212 * (8/1024) => adc count 1 
  //    1 * 0.0033 => 3.3mv, 0.0033 / 2.2 => 1.5 ma
  // 64 * 212 * (8/1024) => adc count 106
  //  106 * 0.0033 => 0.35v,  0.35 / 2.2 => 159 ma

  // brightness ==  1 => 1.5 ma
  // brightness == 14 => 159 ma

  ledAdcTgt = ((u32) expTable[brightness] * 
                     batteryAdc * LED_ADC_TGT_FACTOR) >> 10;
  if(ledAdcTgt > MAX_LED_ADC_TGT) 
     ledAdcTgt = MAX_LED_ADC_TGT;
}

#define PWM_INC 1  // adj pwm this amount each interrupt

// adjust pwm so ledAdc == ledAdcTgt
void adjustPwm(void) {
   static u16 pwmVal = 0;

   u16 ledAdc = handleAdcInt();

   // pwm value only changes by 1 each interrupt
   if(ledAdc < ledAdcTgt && 
      pwmVal < MAX_PWM) pwmVal += PWM_INC;
   if(ledAdc > ledAdcTgt && 
      pwmVal > 0)       pwmVal -= PWM_INC;
   set16(LED_PWM_, pwmVal);
}

// timer interrupts every 64 usecs
@far @interrupt void tim2IntHandler() {
  // used by millis()
  static u16 intCounter = 0;

  // clear all timer 2 int flags
  TIM2->SR1 = 0;

  if(++intCounter == INTS_PER_MS) {
    msCounter++;
    intCounter = 0;
  }
  // run these each interrupt, sort of like a main loop
  adjustPwm();
  inputLoop();
}

void initLed(void) {
  // set timer 2 (IRQ14) interrupt priority to 3 (highest)
  ITC->ISPR4 = (ITC->ISPR4 & ~0x30) | 0x30; // I1I0 is 0b11

  TIM2->CR1 =    // all defaults
    // TIM2_CR1_ARPE   | // 0x80 Auto-Reload Preload Enable mask
    // TIM2_CR1_CMS    | // 0x60 Center-aligned Mode Selection mask
    // TIM2_CR1_DIR    | // 0x10 Direction mask       (dir always up)
    // TIM2_CR1_OPM    | // 0x08 One Pulse Mode mask
    // TIM2_CR1_URS    | // 0x04 Update Request Source mask
    // TIM2_CR1_UDIS   | // 0x02 Update Disable mask
    // TIM2_CR1_CEN    | // 0x01 Counter Enable mask  (enabled at end of init)
    0;

  TIM2->IER =  // timer 2 interrupt enable register
    // TIM2_IER_CC3IE  | // 0x08 Capture/Compare 3 Interrupt Enable mask
    TIM2_IER_CC2IE     | // 0x04 Capture/Compare 2 Interrupt Enable mask
    // TIM2_IER_CC1IE  | // 0x02 Capture/Compare 1 Interrupt Enable mask
    // TIM2_IER_UIE    | // 0x01 Update Interrupt Enable mask
    0;

  // TIM2->SR1 (read-only)
  // TIM2->SR2 (read-only)
  // TIM2->EGR (event gen only)

  TIM2->CCMR3 = 0; // Capture/Compare channel 3 not used

  TIM2->CCMR2 =    // Capture/Compare channel 2 (led pwm)
    // TIM2_CCMR_OCxCE  | // 0x80  Output compare 2 clear enable <not in stm8s.h?>
    0x60                | // TIM2_CCMR_OCxM 0x70 Compare Mode mask (110 => PWM MODE 1)
    // TIM2_CCMR_OCxPE  | // 0x08 Output Compare  x Preload Enable mask
    // TIM2_CCMR_OCxFE  | // 0x04 Output Compare  x Fast Enable mask
    // TIM2_CCMR_CCxS   | // 0x03 Capture/Compare x Selection mask  (0 => output)
    0;

  TIM2->CCMR3 = 
    //TIM2_CCER1_CC2NP  | // 0x80 Capture/Compare 2 Complementary output Polarity mask
    //TIM2_CCER1_CC2NE  | // 0x40 Capture/Compare 2 Complementary output enable mask
    //TIM2_CCER1_CC2P   | // 0x20 Capture/Compare 2 output Polarity (active high)
    TIM2_CCER1_CC2E     | // 0x10 Capture/Compare 2 output enable   (led pwm on)
    //TIM2_CCER1_CC1NP  | // 0x08 Capture/Compare 2 Complementary output Polarity mask
    //TIM2_CCER1_CC1NE  | // 0x04 Capture/Compare 2 Complementary output enable mask
    //TIM2_CCER1_CC1P   | // 0x02 Capture/Compare 1 output Polarity (active high)
    //TIM2_CCER1_CC1E   | // 0x01 Capture/Compare 1 output enable   (left pwm on)
    0;

  TIM2->CCER1 = 0; // Capture/Compare channel 1  not used
 
  // TIM2->TIM2_CNTRH_CNT  // Counter 2 Value (MSB)
  // TIM2->TIM2_CNTRL_CNT  // Counter 2 Value (LSB)
 
  TIM2->PSCR = 0; // Prescaler Value (0 => 16 MHz {default})

  // TIM2->RCR  // repetition counter not used
  // TIM2->BKR  // break input not used
  // TIM2->DTR  // deadtime between comp outputs not used
  // TIM2->OISR // idle control not used

  // Autoreload register sets period for all outputs (1024 => 16 KHz)
  // this doesn't change
  set16(TIM2->ARR, LED_PWM_MAX-1);

  // Capture/Compare Values 
  //     0 =>   0% duty cycle (fet gate voltage at min   0v)
  //  1024 => 100% duty cycle (fet gate voltage at max 3.3v)
  // start with led off
  TIM2->CCR1L = 0;
  TIM2->CCR1H = 0; 
  TIM2->CCR2L = 0;
  TIM2->CCR2H = 0; 

// set pin low initially
  pwm_clr;

// set pin as push-pull output
  pwm_out;

  set16(LED_PWM_, 0);  // start with pwm of zero
  
  // clear all timer 2 int flags
  TIM2->SR1 = 0;
  
  TIM2->EGR  = TIM2_EGR_UG;  // force update of registers
  TIM2->CR1 |= TIM2_CR1_CEN; // 0x01 enable TM2
}
