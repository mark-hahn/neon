#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "input.h"
#include "animation.h"
#include "led.h"

#define TIM2_PRESCALE    0  // clocks at full 16 MHz
#define INTS_PER_MS     16  // pwm freq == 16 KHz (compared to 100 Hz RC)

#define LED_PWM_MAX   1024  // timer rolls over every 64 usecs
#define LED_PWM_L TIM2->CCR2L
#define LED_PWM_H TIM2->CCR2H

// globals
u8  brightness = DEFAULT_BRIGHTNESS;

// convert brightness to 16-bit factor
u16 brightnessFactor(void) {

}

volatile u16 msCounter;

// returns elapsed ms, rolls over every 65 secs
// called by input.c which runs at low priority
u16 millis(void) {
  u16 ms;
  // running at input low priority
  ms = msCounter;
  return ms;
}

u16 ledCurrentTgt = 0;

#define PWM_COUNT_PER_MA 2  // TODO -- guess, measure this

// sets desired led current based on brightness
void setLedCurrentTgt() {
  if(brightness == 0) {
    ledCurrentTgt = 0;
    return;
  }
  if(brightness == MAX_BRIGHTNESS)
    ledCurrentTgt = MAX_CURRENT;
    return;
  }



  // brightness (0..7)  is 2^^(brightness-1) ma,  1/2, 1, .. 64 ma
  if((brightness-1) >= 0)



  if(ledCurrentTgt > MAX_CURRENT) ledCurrentTgt = MAX_CURRENT;
}

// calls handleAdcInt to get led current
// and uses it to adjust pwm to ledCurrentTgt
void adjustPwm(void) {
   static u16 pwmVal = 0;
   u16 current;
   if(ledCurrentTgt == 0) {
     set16(LED_PWM_, 0);
     return;
   }
   if(ledCurrentTgt >= MAX_CURRENT) {
     set16(LED_PWM_, MAX_PWM);
     return;
   }
   current = handleAdcInt(); // already adjusted for bat level
   // pwm value only changes by 1 each interrupt
   if(current < ledCurrentTgt && pwmVal < MAX_PWM) pwmVal++;
   if(current > ledCurrentTgt && pwmVal > 0)       pwmVal--;
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
  animationLoop();
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
