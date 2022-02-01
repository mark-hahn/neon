#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "hdlgts.h"
#include "motors.h"
#include "output.h"
#include "i2c.h"

// command is cccc smmm mmmm mmmm
//   c is opcode, s is sign or flag, and m is magnitude
// opcodes ...
#define PWRLGT_CMD   1
#define HDLGT_CMD_L  2
#define HDLGT_CMD_R  3
#define MOTOR_CMD_L  4
#define MOTOR_CMD_R  5
#define BEEP_CMD     6

#define CMD_FLG_VAL  0x08

// debug: set this using ide 
u8 debugByteCount = 0;
char debugBuf[MAX_RECV_BYTES] = {
  (PWRLGT_CMD   << 4) | CMD_FLG_VAL | 4,   0, // 1 sec flash off
  (HDLGT_CMD_L  << 4) | CMD_FLG_VAL | 4,   0, // flash hdlgt for 1 sec
  (HDLGT_CMD_R  << 4) | CMD_FLG_VAL | 4,   0, // flash hdlgt for 1 sec
  (MOTOR_CMD_L  << 4) | CMD_FLG_VAL | 4,   0, // 100% speed
  (MOTOR_CMD_R  << 4) | CMD_FLG_VAL | 4,   0, // 100% speed
  (BEEP_CMD     << 4) |               0, 100  // 200 ms
};

void initCommand(void) {
  u8 i;
  for(i = 0; i < debugByteCount; i++) 
    recvBuf[i] = debugBuf[i];
}

void commandLoop(void) {
  u8 bytes = numRecvBytes();
  if(debugByteCount) {
    bytes = debugByteCount;
    debugByteCount = 0;
  }
  if(bytes) {
    u8 i;
    for(i=0; i < bytes; i += 2) {
      u8   byte0 = recvBuf[i];
      bool flag  = ((byte0 & 0x08) != 0); // bit flag for sign, blink, or flash
      u16  mag   = ((u16)(byte0 & 0x07) << 8) | recvBuf[i+1];
      switch(byte0 >> 4) {
        case PWRLGT_CMD:   pwrLgt(flag, mag);             break;
        case HDLGT_CMD_L:  setHdlgt(left,  flag, mag);    break;
        case HDLGT_CMD_R:  setHdlgt(right, flag, mag);    break;
        case MOTOR_CMD_L:  setMotorPwm(left,  flag, mag); break;
        case MOTOR_CMD_R:  setMotorPwm(right, flag, mag); break;
        case BEEP_CMD:     beep(mag);                     break;
      }
    }
    cmdPacketProcessed();
  }
}
