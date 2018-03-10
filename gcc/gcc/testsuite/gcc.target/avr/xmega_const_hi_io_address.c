/* Verify that loading the contents of a constant int address in I/O range
   uses two IN instructions with the correct SFR offset for XMEGA*/
/* { dg-do compile } */
/* { dg-options "-Os" } */
/* { dg-skip-if "Only for XMEGAs" { "avr-*-*" } { "*" } { "-mmcu=atxmega*" } } */

void func()
{
    volatile int val = *((int *)0x20);
    *((int *)0x20) = 0xCAFE;

}

/* { dg-final { scan-assembler "\tin r\\d+,0x20\n\tin r\\d+,0x20\\+1" } } */
/* { dg-final { scan-assembler "\tout 0x20,r\\d+\n\tout 0x20\\+1,r\\d+" } } */
