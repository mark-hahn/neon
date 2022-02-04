#ifndef __MAIN__
#define __MAIN__

#define true  1
#define false 0

typedef signed char  i8;
typedef signed short i16;
typedef signed long  i32;

#define set16(_reg,_val) do{_reg##H = (_val) >> 8; _reg##L = (_val) & 0xff;} while(false)
#define get16(_reg) ((_reg##H << 8) | _reg##L)

#define dbgp  pwrlgt_clr; pwrlgt_set  // 120 nsec pulse high on scope
#define dbg1  do { dbgp; } while(false)
#define dbg2  do { dbgp; dbgp; } while(false) // two pulses 1 us apart
#define dbg3  do { dbgp; dbgp; dbgp; } while(false)
#define dbg4  do { dbgp; dbgp; dbgp; dbgp; } while(false)
#define dbg5  do { dbgp; dbgp; dbgp; dbgp; dbgp; } while(false)
#define dbg6  do { dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; } while(false)
#define dbg7  do { dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; } while(false)
#define dbg8  do { dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; dbgp; } while(false)

#define wait() _asm("WFI")

// cannot turn ints on/off in interrupts
#define ints_off _asm("SIM")
#define ints_on  _asm("RIM")

void trace(u8 byte);

#endif
