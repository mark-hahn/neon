#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "adc.h"
#include "animation.h"

// globals
i8   animation = 1;
i8   animSpeed = MAX_SPEED/2
bool flashing  = false;  // overides everything

bool animating = false;

void initAnimVars() {
  // all animations start with led off
  dimFactor = 0;
}

u8 flashCount = 0;

// starts or continues animation by 64 usecs
void doAnim(){
  if(flashing) {

  }
  else if(!animating) {
    initAnimVars();
    animating = true;
  }
  else {
  
    // set dimFactor -- TODO

  }
}

// exits animation mode
void resetAnim(){
  animating = false;
}

void flash(u8 count) {
  totalFlashCount = count;
  flashCount      = 0;
  flashing        = true;
  initAnimVars();
}

// called at power-up
void initAnimation() {

}

void animationLoop() {
  if(flashing) doAnim();
}
