#include "stm8s.h"
#include "main.h"
#include "gpio.h"
#include "output.h"
#include "i2c.h"

#define SLAVE_ADDR 0x20

/*
 recv commands packet...
  first byte of packet is total packet length
    except when first byte is zero, then do power off (set off pin)
  rest of bytes are pairs of bytes, 16-bit cmd/data
  top 4 bits are op-code and last 12 are data (sign magnitude)
  packet can contain 1 to 6 commands
    1: power led flash (period ms)
    2: power led blink (duration ms)
    3: left  headlight (current)
    4: right headlight (current)
    5: left  motor     (pwm)
    6: right motor     (pwm)
    7: beep            (duration ms)
*/
volatile u8   recvBuf[MAX_RECV_BYTES];
volatile bool recvBufReady = false;
volatile u8   recvBufIdx   = 0;
volatile u8   pktLen       = 0;

// num bytes in recvBuf
// non-zero means data buf ready for processing
u8 numRecvBytes(void) {
  if(recvBufReady) return pktLen-1;
  else             return 0;
}

/*
 send return data ...
   2: adc (battery)
   <rest are debug>
   2: adc hdlgt left
   2: adc hdlgt right
   2: hdlgt left  pwm
   2: hdlgt right pwm
*/

#define MAX_SEND_BYTES 6
volatile u8 sendBuf[MAX_SEND_BYTES] = {0};
volatile u8 sendBufIdx = 0;

// main loop done with recv buf
void cmdPacketProcessed() {
  recvBufReady = false;
}

// preSendBuf is copied to sendBuf at send time
volatile u8 preSendBuf[MAX_SEND_BYTES] = {0};

// offsets into sendBuf
#define ADC_DATA_OFS          0
#define HDLGT_ADC_DATA_OFS_L  2
#define HDLGT_ADC_DATA_OFS_R  4

void setAdcSendData(u16 data){
  ints_off;
  preSendBuf[ADC_DATA_OFS]   = data >> 8;
  preSendBuf[ADC_DATA_OFS+1] = data & 0xff;
  ints_on;
}
void setHdlgtAdcDebugDataL(u16 data){
  ints_off;
  preSendBuf[HDLGT_ADC_DATA_OFS_L]   = data >> 8;
  preSendBuf[HDLGT_ADC_DATA_OFS_L+1] = data & 0xff;
  ints_on;
}
void setHdlgtAdcDebugDataR(u16 data){
  ints_off;
  preSendBuf[HDLGT_ADC_DATA_OFS_R]   = data >> 8;
  preSendBuf[HDLGT_ADC_DATA_OFS_R+1] = data & 0xff;
  ints_on;
}

volatile bool recvdFirstByte = false;
volatile u8   dummy;

/////////////////////////////  INTERRUPT ROUTINE  /////////////////////////////
@far @interrupt void i2cIntHandler() {
  u8 err;
  // reading  sr1 clears sr1 flags
	// SR1 and SR3 must be cleared when I2C_SR1_ADDR set (manual)
  u8 sr1 = I2C->SR1;
  u8 sr2 = I2C->SR2;
  u8 sr3 = I2C->SR3;

	if (sr1 & I2C_SR1_ADDR) {		// Our Slave Address has matched
    I2C->CR2 |= I2C_CR2_ACK;	// send ack ever addr and data byte (needed?)
    recvBufIdx    = 0;
    sendBufIdx    = 0;
    recvdFirstByte = false;
  }    
  // note: if recvBufReady still set then error: too many bytes or overrun
	else if(sr3 & I2C_SR3_TRA) {
    // we are in transmission mode
    if(sendBufIdx == 0) {
      // copy from preSendBuf as late as possible
      u8 i;
      for(i = 0; i < MAX_SEND_BYTES; i++)
        sendBuf[i] = preSendBuf[i];
    }

    if(sr2 & I2C_SR2_AF) {
      // NAK:  we've sent all bytes, clear flag
      I2C->SR2 &= ~I2C_SR2_AF;
    }
    else if (sr1 & I2C_SR1_TXE) {
      // Data Register Empty -- load next send byte
      I2C->DR = sendBuf[sendBufIdx++];

  	} else { 
      // Error: TRA but not TXE
    }
  }
  else {
    //we are in reception mode
    if(sr1 & I2C_SR1_RXNE) {
      // Data Register Not empty -- recv byte
      if(!recvdFirstByte) {
        // have first byte
        // packet len is total length including this byte
        pktLen = I2C->DR;
        if(pktLen == 0) {
          ints_off;
          // goodbye cruel world, power off
          off_set;
          while(true);
        }
        recvdFirstByte = true;
      }
      else if(recvBufIdx < MAX_RECV_BYTES && recvBufIdx < pktLen-1) {
        u8 dr = I2C->DR;
        recvBuf[recvBufIdx++] = dr;
      }
      else {
        // error: recvd too many bytes
        dummy = I2C->DR;
      }
    }
    else if(sr1 &  I2C_SR1_STOPF) {
      // got stop bit, recv done
      I2C->CR2 &= ~I2C_SR1_STOPF;  // clear stop bit
      recvBufReady = true;
    }
    else { 
      // Error: receiving (TRA == 0) and no data (RXNE = 0)
    }
  }
	if(sr2 & 0x0F) {
    // an error bit is set
    
		// if(sr2 & I2C_SR2_OVR)  {dbg4;};
		// if(sr2 & I2C_SR2_ARLO) {dbg5;};
		// if(sr2 & I2C_SR2_BERR) {dbg6;};
		// if(sr2 & I2C_SR2_AF)   {dbg7;};

    // clear all error flags
    I2C->SR2 &= ~0x0F;
	}
}

void initI2c(void) {
  // set I2C (IRQ19) software interrupt priority to 2 (next to lowest)
  ITC->ISPR5 = (ITC->ISPR5 & ~0xc0) | 0x00; // 0b00 => level 2

  // I2C->CR1 &= ~I2C_CR1_PE;  // start with peripheral off (PE = 0)

  scl_out_od;  // clk  is open-drain
  sda_out_od;  // data is open-drain

  I2C->CR1 =  // I2C control register 1
    // I2C_CR1_NOSTRETCH  |   // 0x80  Clock Stretching Disable (enabled)
    // I2C_CR1_ENGC       |   // 0x40  General Call Enable (master only)
    // I2C_CR1_PE         |   // 0x01  Peripheral Enable (set at end)
    0;

  I2C->CR2 =  // I2C control register 2
    // I2C_CR2_SWRST  |   // 0x80  Software Reset
    // I2C_CR2_POS    |   // 0x08  Acknowledge (ACK is for current byte)
    I2C_CR2_ACK       |   // 0x04  Acknowledge Enable (ack every byte)
    // I2C_CR2_STOP   |   // 0x02  Stop Generation  (1 => send stop  -- not used)
    // I2C_CR2_START  |   // 0x01  Start Generation (1 => send start -- not used)
    0;

  I2C->FREQR =  // I2C frequency register
    16 | // I2C_FREQR_FREQ 0x3F  Peripheral Clock Frequency(16MHz {>= 4MHz for 400k})
    0;

  I2C->OARL =  // I2C own address register LSB
    (SLAVE_ADDR << 1) | // I2C_OARL_ADD 0xFE  Address [7..1] (addr = 0x20)
    // I2C_OARL_ADD0  | // 0x01  Interface Address bit0 (not used in 7-bit addr)
    0;

  I2C->OARH =  // I2C own address register MSB
    // I2C_OARH_ADDMODE  | // 0x80  Addressing Mode (7-bit addr)
    I2C_OARH_ADDCONF  |    // 0x40  Address Mode Configuration (must be 1)
    // I2C_OARH_ADD      | // 0x06  Interface Address bits [9..8] (for 10-bit addr)
    0;

  I2C->ITR =  // I2C interrupt register -- all type ints enabled
    I2C_ITR_ITBUFEN   |   // 0x04  Buffer Interrupt Enable
    I2C_ITR_ITEVTEN   |   // 0x02  Event Interrupt Enable
    I2C_ITR_ITERREN   |   // 0x01  Error Interrupt Enable
    0;

  // I2C->CCRL       // I2C clock control register low  (master only)
  // I2C->CCRH       // I2C clock control register high (master only)
  // I2C->TRISER     // I2C maximum rise time register  (master only)

  I2C->CR1 |= I2C_CR1_PE;   // 0x01  Peripheral Enable (enable pins)
  I2C->CR2 |= I2C_CR2_ACK;	// send ack every addr and data byte (needed?)

  pwrlgt_out;  // make sure debug pin is ready
}

void i2cLoop(void) {
}
