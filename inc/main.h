#ifndef __MAIN__
#define __MAIN__

#define true  1
#define false 0

typedef signed char  i8;
typedef signed short i16;
typedef signed long  i32;

// byte order is usually important
// this requires that two register names end in H and L
#define set16(_reg,_val) do {                             \
         _reg##H = (_val) >> 8; _reg##L = (_val) & 0xff;} \
        while(false)

#endif
