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
      u16 pwm = (pwmOrDur ? LED_PWM_MAX : 0);
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
      u16 pwm = (pwmOrDur ? LED_PWM_MAX : 0);
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
  set16(TIM2->ARR, LED_PWM_MAX-1);

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

/*

#define MOTLP_PWM_L TIM1->CCR1L
#define MOTLP_PWM_H TIM1->CCR1H
#define MOTLN_PWM_L TIM1->CCR2L
#define MOTLN_PWM_H TIM1->CCR2H
#define MOTRP_PWM_L TIM1->CCR3L
#define MOTRP_PWM_H TIM1->CCR3H
#define MOTRN_PWM_L TIM1->CCR4L
#define MOTRN_PWM_H TIM1->CCR4H

void setMotorPwm(side_t side, bool neg, u16 mag) {
  if(side == left) {
    if(mag == MOTOR_PWM_BRAKE) {
      set16(MOTLP_PWM_, MOTOR_PWM_MAX);
      set16(MOTLN_PWM_, MOTOR_PWM_MAX);
    }
    else {
      if(neg) { // direction reversed on left side
        set16(MOTLP_PWM_, mag);
        set16(MOTLN_PWM_, 0);
      }
      else {
        set16(MOTLP_PWM_, 0);
        set16(MOTLN_PWM_, mag);
      }
    }
  }
  else { // side == right
    if(mag == MOTOR_PWM_BRAKE) {
      set16(MOTRP_PWM_, MOTOR_PWM_MAX);
      set16(MOTRN_PWM_, MOTOR_PWM_MAX);
    }
    else {
      if(!neg) {
        set16(MOTRP_PWM_, mag);
        set16(MOTRN_PWM_, 0);
      }
      else {
        set16(MOTRP_PWM_, 0);
        set16(MOTRN_PWM_, mag);
      }
    }
  }
}

void initMotors(void) {
  TIM1->CR1 =    // all defaults
    // TIM1_CR1_ARPE   | // 0x80 Auto-Reload Preload Enable mask
    // TIM1_CR1_CMS    | // 0x60 Center-aligned Mode Selection mask
    // TIM1_CR1_DIR    | // 0x10 Direction mask       (dir always up)
    // TIM1_CR1_OPM    | // 0x08 One Pulse Mode mask
    // TIM1_CR1_URS    | // 0x04 Update Request Source mask
    // TIM1_CR1_UDIS   | // 0x02 Update Disable mask
    // TIM1_CR1_CEN    | // 0x01 Counter Enable mask  (enabled at end of init)
    0;

  TIM1->CR2 =
    // TIM1_CR2_TI1S   | // 0x80 TI1S Selection mask
    // TIM1_CR2_MMS    | // 0x70 MMS Selection mask
    // TIM1_CR2_COMS   | // 0x04 Capture/Compare Control Update Selection mask
    // TIM1_CR2_CCPC   | // 0x01 Capture/Compare Preloaded Control mask
    0;

  TIM1->SMCR = 0;  // no root-node
  TIM1->ETR  = 0;  // no ext triggers
  TIM1->IER  = 0;  // no interrupts enabled

  // TIM1->SR1 (read-only)
  // TIM1->SR2 (read-only)
  // TIM1->EGR (event gen only)

  TIM1->CCMR1=  // Capture/Compare channel 1  (left motor pos)
    // TIM1_CCMR_OCxCE  | // 0x80  Output compare 2 clear enable <not in stm8s.h?>
    0x60                | // TIM1_CCMR_OCxM 0x70 Output Compare Mode mask (110 => PWM MODE 1)
    // TIM1_CCMR_OCxPE  | // 0x08 Output Compare  x Preload Enable mask
    // TIM1_CCMR_OCxFE  | // 0x04 Output Compare  x Fast Enable mask
    // TIM1_CCMR_CCxS   | // 0x03 Capture/Compare x Selection mask  (0 => output)
    0;

  TIM1->CCMR2 =  // Capture/Compare channel 2  (left motor neg)
    // TIM1_CCMR_OCxCE  | // 0x80  Output compare 2 clear enable <not in stm8s.h?>
    0x60                | // TIM1_CCMR_OCxM 0x70 Output Compare Mode mask (110 => PWM MODE 1)
    // TIM1_CCMR_OCxPE  | // 0x08 Output Compare  x Preload Enable mask
    // TIM1_CCMR_OCxFE  | // 0x04 Output Compare  x Fast Enable mask
    // TIM1_CCMR_CCxS   | // 0x03 Capture/Compare x Selection mask  (0 => output)
    0;

  TIM1->CCMR3=  // Capture/Compare channel 1  (right motor pos)
    // TIM1_CCMR_OCxCE  | // 0x80  Output compare 2 clear enable <not in stm8s.h?>
    0x60                | // TIM1_CCMR_OCxM 0x70 Output Compare Mode mask (110 => PWM MODE 1)
    // TIM1_CCMR_OCxPE  | // 0x08 Output Compare  x Preload Enable mask
    // TIM1_CCMR_OCxFE  | // 0x04 Output Compare  x Fast Enable mask
    // TIM1_CCMR_CCxS   | // 0x03 Capture/Compare x Selection mask  (0 => output)
    0;

  TIM1->CCMR4=  // Capture/Compare channel 1  (right motor neg)
    // TIM1_CCMR_OCxCE  | // 0x80  Output compare 2 clear enable <not in stm8s.h?>
    0x60                | // TIM1_CCMR_OCxM 0x70 Output Compare Mode mask (110 => PWM MODE 1)
    // TIM1_CCMR_OCxPE  | // 0x08 Output Compare  x Preload Enable mask
    // TIM1_CCMR_OCxFE  | // 0x04 Output Compare  x Fast Enable mask
    // TIM1_CCMR_CCxS   | // 0x03 Capture/Compare x Selection mask  (0 => output)
    0;

  TIM1->CCER1 = 
    //TIM1_CCER1_CC2NP  | // 0x80 Capture/Compare 2 Complementary output Polarity mask
    //TIM1_CCER1_CC2NE  | // 0x40 Capture/Compare 2 Complementary output enable mask
    //TIM1_CCER1_CC2P   | // 0x20 Capture/Compare 2 output Polarity (active high)
    TIM1_CCER1_CC2E     | // 0x10 Capture/Compare 2 output enable   (left neg pwm on)
    //TIM1_CCER1_CC1NP  | // 0x08 Capture/Compare 2 Complementary output Polarity mask
    //TIM1_CCER1_CC1NE  | // 0x04 Capture/Compare 2 Complementary output enable mask
    //TIM1_CCER1_CC1P   | // 0x02 Capture/Compare 1 output Polarity (active high)
    TIM1_CCER1_CC1E     | // 0x01 Capture/Compare 1 output enable   (left pos pwm on)
    0;

  TIM1->CCER2 = 
    //TIM1_CCER2_CC4P   | // 0x20 Capture/Compare 4 output Polarity (active high)
    TIM1_CCER2_CC4E     | // 0x10 Capture/Compare 4 output enable   (right neg pwm on)
    //TIM1_CCER2_CC3NP  | // 0x08 Capture/Compare 3 Complementary output Polarity mask
    //TIM1_CCER2_CC3NE  | // 0x04 Capture/Compare 3 Complementary output enable mask
    //TIM1_CCER2_CC3P   | // 0x02 Capture/Compare 3 output Polarity (active high)
    TIM1_CCER2_CC3E     | // 0x01 Capture/Compare 3 output enable   (right pos pwm on)
    0;
    
  // TIM1->TIM1_CNTRH_CNT  // Counter 1 Value (MSB)
  // TIM1->TIM1_CNTRL_CNT  // Counter 1 Value (LSB)
 
  // TIM1->PSCRH = 0;          // Prescaler Value (0 => 16 MHz {default})
  // TIM1->PSCRL = 0;          // Prescaler Value (0 => 16 MHz {default})
  set16(TIM1->PSCR, MOTOR_PRESCALE); // Prescaler Value (7 => 2 MHz)

  // TIM1->RCR // repetition counter not used

  // Autoreload register sets period for all channels (1600 => 10 KHz)
  // this doesn't change
  set16(TIM1->ARR, MOTOR_PWM_MAX-1);

  TIM1->BKR =
    TIM1_BKR_MOE      | // 0x80 Main Output Enable mask
    // TIM1_BKR_AOE   | // 0x40 Automatic Output Enable mask
    // TIM1_BKR_BKP   | // 0x20 Break Polarity mask
    // TIM1_BKR_BKE   | // 0x10 Break Enable mask
    // TIM1_BKR_OSSR  | // 0x08 Off-State Selection for Run mode
    // TIM1_BKR_OSSI  | // 0x04 Off-State Selection for Idle mode
    // TIM1_BKR_LOCK  | // 0x03 Lock Configuration mask
    0;

  TIM1->CCR1L = 0;
  TIM1->CCR1H = 0; 
  TIM1->CCR2L = 0;
  TIM1->CCR2H = 0; 
  TIM1->CCR3L = 0;
  TIM1->CCR3H = 0; 
  TIM1->CCR4L = 0;
  TIM1->CCR4H = 0; 

// set pins as push-pull outputs, low initially
  motlp_clr;
  motln_clr;
  motrp_clr;
  motrn_clr;
  motlp_out;
  motln_out;
  motrp_out;
  motrn_out;  
  
  // TIM1->DTR  = 0;         // deadtime between comp outputs
  TIM1->OISR = 0;            // idle control not needed

  TIM1->EGR  = TIM1_EGR_UG;  // force update of registers
  TIM1->CR1 |= TIM1_CR1_CEN; // 0x01 enable TM1
}

void motorsLoop() {

}
*/