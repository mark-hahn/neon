#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "input.h"
#include "led.h"

#define TIM2_PRESCALE    0  // clocks at full 16 MHz
#define INTS_PER_MS     16  // pwm freq == 16 KHz (compared to 100 Hz RC)

#define LED_PWM_MAX   1024  // timer rolls over every 64 usecs
#define LED_PWM_L TIM2->CCR3L
#define LED_PWM_H TIM2->CCR3H

volatile u16 msCounter = 0;

// returns elapsed ms, rolls over every 65 secs
// called by input.c which runs at low priority
u16 millis(void) {
  return msCounter;
}

// 2**(idx/2)  (except for idx == 0 && idx == 32)
const u16 expTable[33] = {0x0000,
  0x0001, 0x0002, 0x0003, 0x0004, 0x0006, 0x0008, 0x000b, 0x0010, 
  0x0017, 0x0020, 0x002d, 0x0040, 0x005b, 0x0080, 0x00b5, 0x0100, 
  0x016a, 0x0200, 0x02d4, 0x0400, 0x05a8, 0x0800, 0x0b50, 0x1000, 
  0x16a1, 0x2000, 0x2d41, 0x4000, 0x5a82, 0x8000, 0xb505, 0xffff};

#define FLASH_DURATION_MS 300

enum {
  not_flashing,
  flash_active,
  flash_pausing
};

bool flashState = not_flashing;
u16  lastFlashActionMs = 0;
u8   flashes_remaining = 0;

// flash led count+1 times
void flash(u8 count) {
  flashState        = flash_active;
  lastFlashActionMs = millis();
  flashes_remaining = count+1;
}

u16 ledAdcTgt = 0;

// calc led adc value based on batv and brightness
// brightness ==  1 => 1.5 ma
// brightness == 12 => 159 ma
void setLedAdcTgt(u16 batteryAdc) {
  static bool offDueToLight = false;
  u16 lightFactor;
  u16 now = millis();

  if(flashState == flash_active) {
    ledAdcTgt = MAX_LED_ADC_TGT;
    if ((now - lastFlashActionMs) > FLASH_DURATION_MS) {
      lastFlashActionMs = now;
      flashState = flash_pausing;
    }
    return;
  }
  if(flashState == flash_pausing) {
    ledAdcTgt = 0;;
    if ((now - lastFlashActionMs) > FLASH_DURATION_MS) {
      lastFlashActionMs = now;
      if(--flashes_remaining == 0)
           flashState = not_flashing;
      else flashState = flash_active;
    }
    return;
  }

  if(nightLightMode) {
    if(offDueToLight && 
         lightAdc < (nightlightThresh - THRESHOLD_HISTERISIS)){
      offDueToLight = false; 
    }
    if(!offDueToLight && 
         lightAdc > (nightlightThresh + THRESHOLD_HISTERISIS))
      offDueToLight = true;
  }
  else
    // not nightLightMode
    offDueToLight = false;

  if(offDueToLight) {
    ledAdcTgt = 0;
    return;
  }
  // calc factor that dims brightness based on room light
  // factor is lightAdc of 700 - 800, (0.5 to 1)
  if(lightFactor > MAX_LIGHT_ADC)       // 160
    lightFactor = MAX_LIGHT_ADC;
  else if (lightFactor < MIN_LIGHT_ADC) //  10
    lightFactor = MIN_LIGHT_ADC;
  else
    lightFactor = (lightAdc - MIN_LIGHT_ADC);
    
  // rough bits per factor is ...
  //   exptable(6) + battery(8) + adctgt(3) + light(7) => 24
  // total(24) - maxTgt(10) => 14 bits to shift
  // todo - measure actual
  ledAdcTgt = ((u32) expTable[brightness] * batteryAdc * 
                     LED_ADC_TGT_FACTOR   * lightFactor) >> 14;
  if(ledAdcTgt > MAX_LED_ADC_TGT) 
     ledAdcTgt = MAX_LED_ADC_TGT;
}

u16 pwmDebug = 0; // debug
i16 integErr = 0; // debug, should be static in function
u16 sensAdc;

// adjust pwm so ledAdc == ledAdcTgt
void adjustPwm(void) {
  // integral of error, i of pid
  // static i16 integErr = 0;

  if(false && ledAdcTgt == 0) { 
    integErr = 0;
    set16(LED_PWM_, 0);
    return; 
  }
  sensAdc = handleAdcInt();
  // i of pid control, handleAdcInt returns led adc val
  integErr += (ledAdcTgt - sensAdc);


  if(integErr < (MIN_PWM << PWM_DIV))
     integErr = (MIN_PWM << PWM_DIV);

  if(integErr > (MAX_PWM << PWM_DIV))
     integErr = (MAX_PWM << PWM_DIV);

  set16(LED_PWM_, pwmDebug);

  // set16(LED_PWM_, (integErr >> PWM_DIV));
}

// wait 10 ms for everything to stabilize
bool pwrOnStabilizing = true;

// timer interrupts every 64 usecs
@svlreg @far @interrupt void tim2IntHandler() {
  // used by millis()
  static u16 intCounter = 0;
	
  // clear all timer 2 int flags
  TIM2->SR1 = 0;

  if(++intCounter == INTS_PER_MS) {
    msCounter++;
    intCounter = 0;
  }
  if(pwrOnStabilizing && (msCounter > PWR_ON_DELAY_MS)) {
    flash(nightLightMode);
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
 
  TIM2->PSCR = 0; // Prescaler Value (0 => 16 MHz {default})

  // TIM2->RCR  // repetition counter not used
  // TIM2->BKR  // break input not used
  // TIM2->DTR  // deadtime between comp outputs not used
  // TIM2->OISR // idle control not used

  // Autoreload register sets period for all outputs (1024 => 16 KHz)
  // this doesn't change
  set16(TIM2->ARR, LED_PWM_MAX-1);

// set pin low initially
  pwm_clr;

// set pin as push-pull output
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
