
#ifndef _ANIMATION_
#define _ANIMATION_

#include "stm8s.h"
#include "main.h"

enum {
  blinkAnim,     // obnoxious blinking animation, instant rise/fall
  blinkSoftAnim, // blinking animation with rise/fall action speed
  // ... more animation modes
  numAnims
}

extern i8   animation;
extern i8   animSpeed;
extern bool flashing;

void resetAnim();
void doAnim();
void flash(u8 count);

void initAnimation();
void animationLoop();

#endif  // _ANIMATION_

