#ifndef UART_H
#define UART_H

#include <p30Fxxxx.h>
#include <stdio.h>
#include <stdlib.h>

void WriteUART1(unsigned int data);
void RS232_putst(register const char *str);

#endif