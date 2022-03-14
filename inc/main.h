#ifndef __MAIN__
#define __MAIN__

#define true  1
#define false 0

typedef signed char  i8;
typedef signed short i16;
typedef signed long  i32;

#define set16(_reg,_val) do {                             \
         _reg##H = (_val) >> 8; _reg##L = (_val) & 0xff;} \
        while(false)
#define get16(_reg) (((u16) _reg##H << 8) | _reg##L)

#define wait() wfi()

// cannot turn ints on/off in interrupts
#define ints_off disableInterrupts()
#define ints_on  enableInterrupts()

void trace(u8 byte);

#endif
