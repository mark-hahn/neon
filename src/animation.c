#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "input.h"
#include "adc.h"
#include "led.h"
#include "animation.h"


// enum {
//   blinkAnim,     // obnoxious blinking animation, instant rise/fall
//   blinkSoftAnim, // blinking animation with rise/fall action speed
//   // ... more animation modes
//   numAnims       // count of anims above
// }

// globals
i8   animation = 1;
i8   animSpeed = MAX_SPEED/2;
u16  dimFactor = 0;
bool flashing  = false;  // overides everything

bool animating = false;

void initAnimVars(void) {
  // all animations start with led off
  dimFactor = 0;
}

u8 flashCount = 0;

// starts or continues animation by 64 usecs
void doAnim(){
  if(flashing) {

    // set brightness to MIN_BRIGHTNESS or MAX_BRIGHTNESS  -- TODO

  }
  else if(!animating) {
    initAnimVars();
    animating = true;
  }
  else {
  
    // set dimFactor -- TODO

  }
}

void stopAnimation(){
  brightness = MAX_BRIGHTNESS;
  dimFactor  = 0;
  animating  = false;
}

void flash(u8 count) {
  // totalFlashCount = count;
  // flashCount      = 0;
  // flashing        = true;
  // initAnimVars();
}

// called at power-up
void initAnimation() {

}

void animationLoop() {
  if(flashing) doAnim();
}
