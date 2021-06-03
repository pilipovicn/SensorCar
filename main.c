//main.c
// FOSC
#pragma config FOSFPR = XT_PLL4         // Oscillator (XT w/PLL 4x)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

#include <xc.h>
#include "init.h"
#include "uart.h"
#include <math.h>

#define FORWARD_MOVE()({LATFbits.LATF0 = 1; LATBbits.LATB12 = 0; LATBbits.LATB9 = 0; LATBbits.LATB10 = 1;})
#define BACKWARD_MOVE()({LATFbits.LATF0 = 0; LATBbits.LATB12 = 1; LATBbits.LATB9 = 1; LATBbits.LATB10 = 0;})
#define STOP_MOVE()({LATFbits.LATF0 = 0; LATBbits.LATB12 = 0; LATBbits.LATB9 = 0; LATBbits.LATB10 = 0;})
#define RIGHT_TURN()({LATFbits.LATF0 = 0; LATBbits.LATB12 = 1; LATBbits.LATB9 = 0; LATBbits.LATB10 = 1;})
#define LEFT_TURN()({LATFbits.LATF0 = 1; LATBbits.LATB12 = 0; LATBbits.LATB9 = 1; LATBbits.LATB10 = 0;})
#define LEFT_TURN_GYRATE()({LATFbits.LATF0 = 1; LATBbits.LATB12 = 0; LATBbits.LATB9 = 0; LATBbits.LATB10 = 0;})
#define STOP_TURN()({LATBbits.LATB9 = 0; LATBbits.LATB10 = 0;})

int falling_edgeFW = 0;
int falling_edgeSD = 0;
int t1=0;
int t2=0;
int t3=0, t4=0;
volatile float distFW, distSD;
int timeFW=0, timeSD;
char message[100];
char test[100];
int next_frame = 1;
int fw_thr = 8;
int sd_thr = 7;
int readyFW = 0;
int readySD = 0;

void turn_manuevre(){
    LEFT_TURN_GYRATE();
    __delay_ms(2100);
    FORWARD_MOVE();
    __delay_ms(600);
    STOP_MOVE();
    
}

int main(){
    init_pins();
    InitTimer3();
    InitTimer1();
    InitTimer2();
    InitUART1();
    InitHC_SR04();
    T1CONbits.TON = 1;
    
    while(1){
        //trigger_distance();
        //while(!readyFW && !readySD);
        if(distSD < 4){
            FORWARD_MOVE();
            __delay_ms(30);
            LEFT_TURN();
        }else if(distSD > 6){
            FORWARD_MOVE();
            __delay_ms(30);
            RIGHT_TURN();
        }else{
            if(distFW > 11.5){
                
                FORWARD_MOVE();
            }else{
                
                turn_manuevre();
               
            }
        }
        
        sprintf(message, "Dist FW: %.1f | Dist SD: %.1f \n\r", distFW, distSD);
        RS232_putst(message);
        
    }

    return 1;
}

void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void){
	// Clear interrupt flag
	IFS0bits.U1RXIF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _IC1Interrupt(void){
    if(!falling_edgeFW){
        t1 = IC1BUF;
        falling_edgeFW = 1;
    }else{
        t2 = IC1BUF;
        falling_edgeFW = 0;
        readyFW = 1;
        //sprintf(test, "t1: %X | t2: %X\n", t1, t2);
        //RS232_putst(test);
        if(t2>t1){
            timeFW = t2 - t1;
        }else{
            timeFW = (0xFFFF - t1)+t2;
        }
        distFW = (34300 * 0.0000064 * timeFW)/2;
        
    }

    IFS0bits.IC1IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _IC2Interrupt(void){
      if(!falling_edgeSD){
        t3 = IC2BUF;
        falling_edgeSD = 1;
    }else{
        t4 = IC2BUF;
        falling_edgeSD = 0;
        readySD = 1;
        if(t4>t3){
            timeSD = t4 - t3;
        }else{
            timeSD = (0xFFFF - t3) + t4;
        }
        distSD = (34300 * 0.0000064 * timeSD)/2;
    }

    IFS0bits.IC2IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void){
    TRIG_FW = 1;
    TRIG_SD = 1;
    TMR1=0;
    while(TMR1 < 3);
    TRIG_FW = 0;
    TRIG_SD = 0;
    
	IFS0bits.T1IF = 0;   
}


void __attribute__((__interrupt__)) _T2Interrupt(void){
   	TMR2 = 0;
    IFS0bits.T2IF = 0;
}