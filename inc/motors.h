#ifndef __MOTORS__
#define __MOTORS__

// set alternative function register 0, afr0
// left motor:     tim1_1, tim1_2
// left headlight: tim2_1

#include "main.h"

#define MOTOR_PRESCALE      31  // TIM1 prescale of /32 -> 500 kHz clk
#define MOTOR_PWM_MAX     1024  // period 2ms (500 Hz)
#define MOTOR_PWM_BRAKE 0x07ff  // pwm flag value to indicate brake

void setMotorPwm(side_t side, bool neg, u16 mag);

void initMotors(void);
void motorsLoop(void);

#endif  // __MOTORS__

