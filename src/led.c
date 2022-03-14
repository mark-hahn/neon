#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "input.h"
#include "led.h"

u16 msCounter = 0;

// returns elapsed ms, rolls over every 65 secs
// called by input.c
u16 millis(void) {
  return msCounter;
}

// 2**round(idx/2)  (except for idx == 0 & 32)
const u16 expTable[33] = {0x0000,
  0x0001, 0x0002, 0x0003, 0x0004, 0x0006, 0x0008, 0x000b, 0x0010, 
  0x0017, 0x0020, 0x002d, 0x0040, 0x005b, 0x0080, 0x00b5, 0x0100, 
  0x016a, 0x0200, 0x02d4, 0x0400, 0x05a8, 0x0800, 0x0b50, 0x1000, 
  0x16a1, 0x2000, 0x2d41, 0x4000, 0x5a82, 0x8000, 0xb505, 0xffff};

u16 ledAdcTgt = 0;

// calc led adc value based on batv and brightness
// dayBrightness   ==  1  =>    3 ma
// dayBrightness   ==  9  =>  250 ma
// nightBrightness ==  1  =>  0.5 ma
// nightBrightness ==  9  =>   12 ma
void setLedAdcTgt(u16 batteryAdc) {
  static bool offDueToLight = false;

  u16 now = millis();
  i32 lightFactor = 160;

  if(nightMode) {
    if(BUTTON_DOWN) {
      offDueToLight = (lightAdc > (nightlightThresh));
    }
    else if(inputActive) {
      offDueToLight = false;
    }
    else if(offDueToLight && 
         lightAdc < (nightlightThresh - THRESHOLD_HYSTERISIS)){
      offDueToLight = false; 
    }
    else if(!offDueToLight && 
         lightAdc > (nightlightThresh + THRESHOLD_HYSTERISIS))
      offDueToLight = true;
      
    if(offDueToLight) {
      ledAdcTgt = 0;
      return;
    }
  }
  else
    offDueToLight = false;

  if(nightMode) lightFactor = MAX_LIGHT_FACTOR; 
  else {
    lightFactor = lightAdc;
    if(lightFactor > MAX_LIGHT_FACTOR)       // 240
      lightFactor = MAX_LIGHT_FACTOR;
    else if (lightFactor < MIN_LIGHT_FACTOR) //  10
      lightFactor = MIN_LIGHT_FACTOR;
  }
  
  ledAdcTgt = ((u32) expTable[nightMode ? nightBrightness : dayBrightness] 
                       * batteryAdc * lightFactor) >> ADC_TGT_FACTOR; 

  boost_setto(!nightMode);
}

// adjust pwm so ledAdc == ledAdcTgt
void adjustPwm(void) {
  static i32 integErr = 0;
  
  u16 ledAdc    = 0;
  i32 ledAdcErr = 0;
  i32 IContrib  = 0;
  i32 PContrib  = 0;
  i16 pwm       = 0;
  
  ledAdc = handleAdc();
  ledAdcErr = ((i32) ledAdc - (i32) ledAdcTgt);
	
  // integral of error, I of PID
  integErr += -ledAdcErr;
  if(integErr < MIN_INTEGRAL) integErr = MIN_INTEGRAL;
  if(integErr > MAX_INTEGRAL) integErr = MAX_INTEGRAL;
	IContrib = integErr >> Ik;
	
  // error, P of PID
	PContrib = -(ledAdcErr << Pk);
	
  pwm = (IContrib + PContrib);

  if(pwm > MAX_PWM) pwm = MAX_PWM;
  if(pwm < MIN_PWM) pwm = MIN_PWM;

  set16(LED_PWM_, pwm);
}

// wait 10 ms for everything to stabilize
bool pwrOnStabilizing = true;

// timer interrupts every 128 usecs
@svlreg @far @interrupt void tim2IntHandler() {
  // used by millis()
  static u16 intCounter = 0;
	
  // clear all timer 2 int flags
  TIM2->SR1 = 0;

  // msCounter is returned by millis()
  if(++intCounter == INTS_PER_MS) {
    msCounter++;
    intCounter = 0;
  }

  if(pwrOnStabilizing && (msCounter > PWR_ON_DELAY_MS)) {
    pwrOnStabilizing = false;
  }
	if(pwrOnStabilizing) return;

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
    TIM2_IER_CC3IE  |    // 0x08 Capture/Compare 3 Interrupt Enable mask
    // TIM2_IER_CC2IE  | // 0x04 Capture/Compare 2 Interrupt Enable mask
    // TIM2_IER_CC1IE  | // 0x02 Capture/Compare 1 Interrupt Enable mask
    // TIM2_IER_UIE    | // 0x01 Update Interrupt Enable mask
    0;

  // TIM2->SR1 (read-only)
  // TIM2->SR2 (read-only)
  // TIM2->EGR (event gen only)

  // TIM2->CCMR1    // Capture/Compare channel 1 not used
  // TIM2->CCMR2    // Capture/Compare channel 2 not used

  TIM2->CCMR3 =     // Capture/Compare channel 3  (led pwm)
    // TIM2_CCMR_OCxCE  | // 0x80  Output compare 3 clear enable <not in stm8s.h?>
    0x60                | // TIM2_CCMR_OCxM 0x70 Compare Mode mask (110 => PWM MODE 1)
    // TIM2_CCMR_OCxPE  | // 0x08 Output Compare  3 Preload Enable mask
    // TIM2_CCMR_CCxS   | // 0x03 Capture/Compare 3 Selection mask  (0 => output)
    0;

  // TIM2->CCER1          // cc1 & cc2 output control,  not used

  TIM2->CCER2 =           // Capture/Compare channel 3   (led pwm)
    // TIM2_CCER2_CC3P  | // 0x02 Capture/Compare 3 output Polarity (active high)
    TIM2_CCER2_CC3E     | // 0x01 Capture/Compare 3 output enable
    0;

  // TIM2->TIM2_CNTRH_CNT  // Counter 2 Value (MSB)
  // TIM2->TIM2_CNTRL_CNT  // Counter 2 Value (LSB)
 
  TIM2->PSCR = TIM2_PRESCALE; // 1 => 8 MHz => 8 Khz rollover

  // TIM2->RCR  // repetition counter not used
  // TIM2->BKR  // break input not used
  // TIM2->DTR  // deadtime between comp outputs not used
  // TIM2->OISR // idle control not used

  // Autoreload register sets period for all outputs (1024 => 16 KHz)
  // this doesn't change
  set16(TIM2->ARR, LED_PWM_MAX-1);

// set boost pin as push-pull output and clr
  boost_set;
  boost_out;

// set pwm pin as push-pull output and clr
  pwm_clr;
  pwm_out;

  // Capture/Compare Values 
  //     0 =>   0% duty cycle (fet gate voltage at min   0v)
  //  1024 => 100% duty cycle (fet gate voltage at max 3.3v)
  // start with led off
  set16(LED_PWM_, 0);  // start with pwm of zero
  
  // clear all timer 2 int flags
  TIM2->SR1 = 0;
  
  TIM2->EGR  = TIM2_EGR_UG;  // force update of registers
  TIM2->CR1 |= TIM2_CR1_CEN; // 0x01 enable TM2
}
