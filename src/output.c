
#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "clock.h"
#include "output.h"

u16 beepDur = 0;

// beep for durationMs
// 0 => cancel
void beep(u16 durationMs) {
  if(durationMs) beep_set; // start beep
  beepDur = durationMs;
}

#define PWR_LED_BLINK_OFF_MS 50

u16  pwrLedOnDur = 0;
bool pwrLedBlink = false;

// blink false:  light flashes on/off with durationMs period, 50% duty cycle
// blink true:   light blinks off for 50ms once every durationMs
// durationMs 0: light constantly on
void pwrLgt(bool blink, u16 durationMs) {
  pwrLedBlink = blink;
  pwrLedOnDur = durationMs;
}

void initOutput(void) {
  beep_clr;    // start with no beep
  beep_out;    // beep pin is output push-pull (not using beep freq hw)

  pwrlgt_set;  // light on by default, only goes off when flashing
  pwrlgt_out;  // power light pin is output push-pull

  off_clr;     // off pin is set in i2c.c, not here
  off_out;     // off pin is output push-pull
}

void outputLoop(void) {
  u16 now = millis();
  static u32 lastMsOut = 0;
  if(now != lastMsOut) {
    // this happens ~ every 1 ms

    static u16 ledCounter = 0;
    if(pwrLedOnDur) {
      if(!pwrLedBlink) { // blink not set
        if(++ledCounter >= pwrLedOnDur/2) {
          ledCounter = 0;
          pwrlgt_toggle;
        }
      }
      else { // blink set
        if(++ledCounter >= pwrLedOnDur) ledCounter = 0;
        if(ledCounter < PWR_LED_BLINK_OFF_MS) pwrlgt_clr;
        else                                  pwrlgt_set;
      }
    }
    else pwrlgt_set;

    if(beepDur && (--beepDur == 0)) beep_clr;

    lastMsOut = now;
  }
}
