#ifndef __I2C__
#define __I2C__

#include "stm8s.h"

#define MAX_RECV_BYTES 14

extern volatile u8 recvBuf[MAX_RECV_BYTES];

// recvBuf length
// zero means recv buf not ready
u8 numRecvBytes(void);

// called when main loop done with recv buf
void cmdPacketProcessed(void);

// put battery adc value in send buf
void setAdcSendData(u16 data);

// these are for debug
void setHdlgtAdcDebugDataL(u16 data);
void setHdlgtAdcDebugDataR(u16 data);
void setHdlgtDebugDataL(u16 data);
void setHdlgtDebugDataR(u16 data);

@far @interrupt void i2cIntHandler();

void initI2c(void);
void i2cLoop(void);
 
#endif
