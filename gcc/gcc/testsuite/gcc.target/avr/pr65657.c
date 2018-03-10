/* { dg-do run } */
/* { dg-options "-Os" } */

/* When cfgexpand expands the call to foo, it
   assigns registers R22:R23 to 0xABCD and R24
   to *x. Because x is a __memx address space
   pointer, the dereference results in an RTL
   insn that clobbers R22 among other 
   registers (xload<mode>_A).

   Call expansion does not take this into account
   and ends up clobbering R22 set previously to 
   hold part of the second argument.
*/

#include <stdlib.h>

void __attribute__((noinline))
    __attribute__((noclone))
foo (char c, volatile unsigned int d)
{
    if (d != 0xABCD)
      abort();
    if (c != 'A')
        abort();
}

void __attribute__((noinline))
    __attribute__((noclone))
readx(const char __memx *x)
{
    foo (*x, 0xABCD);
}

const char __memx arr[] = { 'A' };
int main()
{
   readx (arr); 
}
