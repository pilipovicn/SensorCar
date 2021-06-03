#ifndef INIT_H
#define INIT_H
#define _XTAL_FREQ      10000000
#define XTFREQ          10000000             // On-board Crystal frequency
#define PLLMODE         4              // On-chip PLL setting (Fosc)
#define FCY             (XTFREQ*PLLMODE)/4  // Instruction Cycle Frequency (Fosc/2)
#define FPWM		9600
#define BAUDRATE         ((FCY/FPWM)/16)-1

#include <p30Fxxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include <libpic30.h>


// ******************************************************************************************* //
// Defines to simply UART's baud rate generator (BRG) regiser
// given the osicllator freqeuncy and PLLMODE.


#define TRIG_FW LATDbits.LATD3
#define TRIG_SD LATDbits.LATD2
#define ECHO_FW PORTDbits.RD8
#define ECHO_SD PORTDbits.RD9


//#define DELAY_105us asm volatile ("REPEAT, #3869"); Nop(); // 105uS delay

void init_pins();
void InitUART1();
void InitTimer1();
void InitTimer2();
void InitTimer3();
void InitTimer4();
void InitHC_SR04();



#endif