#include "uart.h"

void WriteUART1(unsigned int data){
	while(!U1STAbits.TRMT);

    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}

void RS232_putst(register const char *str){
    while((*str)!=0){
        WriteUART1(*str);
        if (*str==13) WriteUART1(10);
        if (*str==10) WriteUART1(13);
        str++;
    }
}