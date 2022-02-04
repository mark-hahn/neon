
#ifndef _ANIMATION_
#define _ANIMATION_

#include "stm8s.h"
#include "main.h"

extern u16  dimFactor;
extern i8   animation;
extern i8   animSpeed;
extern bool flashing;

void stopAnimation(void);
void doAnim(void);
void flash(u8 count);

void initAnimation(void);
void animationLoop(void);

#endif  // _ANIMATION_

