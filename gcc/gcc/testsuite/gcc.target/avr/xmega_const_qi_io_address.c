/* Verify that loading the contents of a constant address in I/O range
   uses the IN instruction with the correct SFR offset for XMEGA*/
/* { dg-do compile } */
/* { dg-options "-Os" } */
/* { dg-skip-if "Only for XMEGAs" { "avr-*-*" } { "*" } { "-mmcu=atxmega*" } } */

void func()
{
    volatile char val = *((char *)0x20);
    *((char *)0x20) = 42;
}

/* { dg-final { scan-assembler "\tin r\\d+,0x20" } } */
/* { dg-final { scan-assembler "\tout 0x20,r\\d+" } } */
