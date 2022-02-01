#include "stm8s.h"
#include "gpio.h"
#include "clock.h"
#include "adc.h"
#include "i2c.h"
#include "hdlgts.h"

// set alternative function register 0, afr0
// left motor:     tim1_1, tim1_2
// left headlight: tim2_1

#define HDLL_H TIM2->CCR1H
#define HDLL_L TIM2->CCR1L
#define HDLR_H TIM2->CCR2H
#define HDLR_L TIM2->CCR2L

u16 hdlgtCounterL = 0;
u16 hdlgtCounterR = 0;

// if flash not set, set headlight TIM2 pwm to pwmOrDur
// if flash set, run full power for pwmOrDur ms
void setHdlgt(side_t side, bool flash, u16 pwmOrDur) {
  if(side == left) {
    if(!flash) {
      // just set pwm
      set16(HDLL_, pwmOrDur);
      hdlgtCounterL = 0;
    }
    else {
      u16 pwm = (pwmOrDur ? HDLGT_PWM_MAX : 0);
      set16(HDLL_, pwm);
      hdlgtCounterL = pwmOrDur;
    }
  }
  else { // side == right
    if(!flash) {
      // just set pwm
      set16(HDLR_, pwmOrDur);
      hdlgtCounterR = 0;
    }
    else {
      u16 pwm = (pwmOrDur ? HDLGT_PWM_MAX : 0);
      set16(HDLR_, pwm);
      hdlgtCounterR = pwmOrDur;
    }
  }
}

void initHdlgts(void) {
  TIM2->CR1 =    // all defaults
    // TIM2_CR1_ARPE   | // 0x80 Auto-Reload Preload Enable mask
    // TIM2_CR1_CMS    | // 0x60 Center-aligned Mode Selection mask
    // TIM2_CR1_DIR    | // 0x10 Direction mask       (dir always up)
    // TIM2_CR1_OPM    | // 0x08 One Pulse Mode mask
    // TIM2_CR1_URS    | // 0x04 Update Request Source mask
    // TIM2_CR1_UDIS   | // 0x02 Update Disable mask
    // TIM2_CR1_CEN    | // 0x01 Counter Enable mask  (enabled at end of init)
    0;

  TIM2->IER  = 0;  // no interrupts enabled

  // TIM2->SR1 (read-only)
  // TIM2->SR2 (read-only)
  // TIM2->EGR (event gen only)

  TIM2->CCMR1=  // Capture/Compare channel 1  (left hdlgt)
    // TIM2_CCMR_OCxCE  | // 0x80  Output compare 2 clear enable <not in stm8s.h?>
    0x60                | // TIM2_CCMR_OCxM 0x70 Output Compare Mode mask (110 => PWM MODE 1)
    // TIM2_CCMR_OCxPE  | // 0x08 Output Compare  x Preload Enable mask
    // TIM2_CCMR_OCxFE  | // 0x04 Output Compare  x Fast Enable mask
    // TIM2_CCMR_CCxS   | // 0x03 Capture/Compare x Selection mask  (0 => output)
    0;

  TIM2->CCMR2 =  // Capture/Compare channel 2  (right hdlgt)
    // TIM2_CCMR_OCxCE  | // 0x80  Output compare 2 clear enable <not in stm8s.h?>
    0x60                | // TIM2_CCMR_OCxM 0x70 Output Compare Mode mask (110 => PWM MODE 1)
    // TIM2_CCMR_OCxPE  | // 0x08 Output Compare  x Preload Enable mask
    // TIM2_CCMR_OCxFE  | // 0x04 Output Compare  x Fast Enable mask
    // TIM2_CCMR_CCxS   | // 0x03 Capture/Compare x Selection mask  (0 => output)
    0;

  TIM2->CCMR3 = 0;  // Capture/Compare channel 3 not used

  TIM2->CCER1 = 
    //TIM2_CCER1_CC2NP  | // 0x80 Capture/Compare 2 Complementary output Polarity mask
    //TIM2_CCER1_CC2NE  | // 0x40 Capture/Compare 2 Complementary output enable mask
    //TIM2_CCER1_CC2P   | // 0x20 Capture/Compare 2 output Polarity (active high)
    TIM2_CCER1_CC2E     | // 0x10 Capture/Compare 2 output enable   (right pwm on)
    //TIM2_CCER1_CC1NP  | // 0x08 Capture/Compare 2 Complementary output Polarity mask
    //TIM2_CCER1_CC1NE  | // 0x04 Capture/Compare 2 Complementary output enable mask
    //TIM2_CCER1_CC1P   | // 0x02 Capture/Compare 1 output Polarity (active high)
    TIM2_CCER1_CC1E     | // 0x01 Capture/Compare 1 output enable   (left pwm on)
    0;

  TIM2->CCER2 = 0; // Capture/Compare channel 3 not used
 
  // TIM2->TIM2_CNTRH_CNT  // Counter 2 Value (MSB)
  // TIM2->TIM2_CNTRL_CNT  // Counter 2 Value (LSB)
 
  TIM2->PSCR = 0; // Prescaler Value (0 => 16 MHz {default})

  // TIM2->RCR  // repetition counter not used
  // TIM2->BKR  // break input not used
  // TIM2->DTR  // deadtime between comp outputs not used
  // TIM2->OISR // idle control not used

  // Autoreload register sets period for all outputs (1024 => 16 KHz)
  // this doesn't change
  set16(TIM2->ARR, HDLGT_PWM_MAX-1);

  // Capture/Compare Values 
  //     0 =>   0% duty cycle (fet gate voltage at min   0v)
  //  1024 => 100% duty cycle (fet gate voltage at max 3.3v)
  // start with hdlgts off
  TIM2->CCR1L = 0;
  TIM2->CCR1H = 0; 
  TIM2->CCR2L = 0;
  TIM2->CCR2H = 0; 

// set pins low initially
  hdlgtl_clr;
  hdlgtr_clr;

// set pins as push-pull outputs
  hdlgtl_out;
  hdlgtr_out;
  
  TIM2->EGR  = TIM2_EGR_UG;  // force update of registers
  TIM2->CR1 |= TIM2_CR1_CEN; // 0x01 enable TM2
}

void hdlgtsLoop(void) {
  static u16 lastMsHdl = 0;
  u16 now = millis();
  if(now != lastMsHdl) {
    // runs once every ms
    lastMsHdl = now;
    if(hdlgtCounterL && (--hdlgtCounterL == 0)) set16(HDLL_, 0);
    if(hdlgtCounterR && (--hdlgtCounterR == 0)) set16(HDLR_, 0);
  }
}
