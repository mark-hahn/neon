#include "stm8s.h"
#include "main.h"
#include "gpio.h"

// neon operation modes
#MODE_NORMAL 0 // always on, knob sets brightness

// button click (just pressed)
void buttonPress() {

}

// forward rotation click
void encoderCW() {

}

// backward rotation click
void encoderCCW() {

}

// interrupts every button or encoder pin change (ports C and D)
@far @interrupt void inputIntHandler() {
  static bool lastBtnDown = false;
  static bool lastActiveA = false; // used as clock
  bool btnDown;
  bool activeA;

  // check button
  btnDown = button_lvl;         // need debouncing ???
  if(!lastBtnDown && btnDown) buttonPress;
  lastBtnDown = btnDown;
  
  // check encoder
  activeA = enca_lvl;           // need debouncing ???
  if(!lastActiveA && activeA) {
    // have positive clock edge
    // check direction
    if(encb_lvl) encoderCW();  // clockwise
    else         encoderCCW(); // counter-clockwise
  }
  lastActiveA = activeA;
}

void initInput(void) {
  // set button (IRQ6)  interrupt priority to 1 (lowest)
  ITC->ISPR2 = (ITC->ISPR2 & ~0x30) | 0x10; // I1I0 is 0b01 

  // set encoder (IRQ5) interrupt priority to 2 (middle)
  ITC->ISPR2 = (ITC->ISPR2 & ~0x0c) | 0x00; // I1I0 is 0b00

  // button and encoder external interrupts
  button_in_int;
  enca_in_int;
  encb_in_int;

  // pins interrupt on both edges
  EXTI_CR1 = 0xff;
}

// called every timer interrupt (64 usecs) from led.c
// runs at highest interrupt priority
void inputLoop(void) {

}

