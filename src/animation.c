#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "input.h"
#include "adc.h"
#include "led.h"
#include "animation.h"

// globals
u8 animation = 0;
u8 animSpeed = DEFAULT_ANIM_SPEED;

bool animating = false;
bool flashing  = false;  // overides everything

void initAnimVars(void) {
  // all animations start with led off
  dimness = 0;
}

u8 flashCount = 0;

// starts or continues animation by 64 usecs
void doAnim(void){
  if(flashing) {

    // set brightness to MIN_BRIGHTNESS or MAX_BRIGHTNESS  -- TODO

  }
  else if(!animating) {
    initAnimVars();
    animating = true;
  }
  else {
  
    // set dimness -- TODO

  }
}

void stopAnimation(void){
  brightness = MAX_BRIGHTNESS;
  dimness  = 0;
  animating  = false;
}

void flash(u8 count) {
  // totalFlashCount = count;
  // flashCount      = 0;
  // flashing        = true;
  // initAnimVars();
}

void animationLoop(void) {
  if(flashing) doAnim();
}
