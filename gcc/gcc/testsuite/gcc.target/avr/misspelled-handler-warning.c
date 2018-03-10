/* Test warning emitted for functions with nmi attribute that do
 * not start with __vector */
/* { dg-do compile } */


void __attribute__((interrupt)) interrupt_fun() /* { dg-warning "'interrupt_fun' appears to be a misspelled interrupt handler" } */
{}

void __attribute__((signal)) signal_fun() /* { dg-warning "'signal_fun' appears to be a misspelled signal handler" } */
{}

void __attribute__((nmi)) nmi_fun() /* { dg-warning "'nmi_fun' appears to be a misspelled nmi handler" } */
{}
