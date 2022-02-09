
#ifndef _ANIMATION_
#define _ANIMATION_

#include "stm8s.h"
#include "main.h"

#define MAX_SPEED          10 
#define DEFAULT_ANIM_SPEED  5

enum {
  blinkAnim,     // obnoxious blinking animation, instant rise/fall
  blinkSoftAnim, // blinking animation with rise/fall action speed
  // ... more animation modes
  numAnims       // count of anims above
};

extern u8   animation;

// speed (0..10) is 2^^(speed-4) secs,  1/16..64 secs per action
extern u8   animSpeed;

void stopAnimation(void);
void doAnim(void);
void flash(u8 count);

void initAnimation(void);
void animationLoop(void);

#endif  // _ANIMATION_

