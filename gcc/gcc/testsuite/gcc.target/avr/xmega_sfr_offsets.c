/* Verify that SFR offsets for XMEGAs do not have the 0x20 offset 
   and that they are saved on entry, restored on exit for an interrupt
   function  */
/* { dg-do compile } */
/* { dg-options "-Os" } */
/* { dg-skip-if "Only for XMEGAs" { "avr-*-*" } { "*" } { "-mmcu=atxmega128a1" } } */

void __attribute__((interrupt)) __vector_1()
{
}

/* { dg-final { scan-assembler "__SREG__ = 0x3f" } } */
/* { dg-final { scan-assembler "__RAMPD__ = 0x38" } } */
/* { dg-final { scan-assembler "\tin r0,__SREG__" } } */
/* { dg-final { scan-assembler "\tin r0,__RAMPD__" } } */
/* { dg-final { scan-assembler "\tpop r0\n\tout __SREG__,r0" } } */
/* { dg-final { scan-assembler "\tpop r0\n\tout __RAMPD__,r0" } } */

