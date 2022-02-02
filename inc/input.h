#ifndef __INPUT__
#define __INPUT__

#include "stm8s.h"
#include "main.h"

@far @interrupt void inputIntHandler();

void initInput(void);
void inputLoop(void);
 
#endif




