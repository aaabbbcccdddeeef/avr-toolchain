/* Verify that XMEGA interrupts don't have a cli or sei
   and that SPL is written before SPH*/
/* { dg-do compile } */
/* { dg-options "-Os" } */
/* { dg-skip-if "Only for XMEGAs" { "avr-*-*" } { "*" } { "-mmcu=atxmega*" } } */

void __attribute__((interrupt)) __vector_1()
{
    volatile int w = 19, x = 20, y = 30, z = 42;
}

/* { dg-final { scan-assembler-not "\tcli" } } */
/* { dg-final { scan-assembler "\tout __SP_L__,r\\d+\n\tout __SP_H__,r\\d+" } } */

