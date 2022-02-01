#ifndef __MAIN__
#define __MAIN__

#define true  1
#define false 0

// set alternative function register 0, afr0
// left motor:     tim1_1, tim1_2
// left headlight: tim2_1

enum { left, right };
typedef uint8_t side_t;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;

#define set16(_reg,_val) do{_reg##H = (_val) >> 8; _reg##L = (_val) & 0xff;} while(false)

#define dbgp  pwrlgt_clr; pwrlgt_set  // 120 nsec pulse high on scope
#define dbg1  do { dbgp; } while(false)
#define dbg2  do { dbgp; dbgp; } while(false) // two pulses 1 us apart
#define dbg3  do { dbgp; dbgp; dbgp; } while(false)
#define dbg4  do { dbgp; dbgp; dbgp; dbgp; } while(false)
#define dbg5  do { dbgp; dbgp; dbgp; dbgp; dbgp; } while(false)
#define dbg6  do { dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; } while(false)
#define dbg7  do { dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; } while(false)
#define dbg8  do { dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; } while(false)

// cannot turn ints on/off in interrupts
#define ints_off _asm("SIM")
#define ints_on  _asm("RIM")

void trace(u8 byte);

#endif
