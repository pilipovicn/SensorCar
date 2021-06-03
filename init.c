#include "init.h"


void init_pins(){
    TRISDbits.TRISD0 = 0; // EN2 Motor
    TRISDbits.TRISD1 = 0; // EN1 Motor
    TRISBbits.TRISB9 = 0; // In1/2
    TRISBbits.TRISB10 = 0; 
    TRISFbits.TRISF0 = 0; // Rear wheel drive direction In1/2
    TRISBbits.TRISB12 = 0;   
}

void InitUART1(void){
    U1BRG=BAUDRATE;           // Set baud rate to ~1200
    U1MODEbits.ALTIO=0;     // Use main UART
    IEC0bits.U1RXIE=1;      // Enable Interrupts on Rx
    U1STA&=0xfffc;          // Init Status Reg
    U1MODEbits.UARTEN=1;    // Enable module
    U1STAbits.UTXEN=1;      // Enable Transmits
}

void InitTimer1()
{
	// Clear Timer value (i.e. current tiemr value) to 0
	TMR1 = 0;
        
	T1CONbits.TCS = 0; //source is Fcy
	T1CONbits.TCKPS = 2; //1:64
        //Set PR1 to 60ms
	//PR1 = 37500;
    PR1 = 9375;

	// Clear Timer 1 interrupt flag. This allows us to detect the
	// first interupt.
	IFS0bits.T1IF = 0;

	// Enable the interrupt for Timer 1
	IEC0bits.T1IE = 1;
}

void InitTimer2() {

	PR2 = 4999;             // Set Timer 3 period ---- 10 000 000 -> Perioda kontrolera = 1.0e-7*5000 = 0.0005 = 2.0kHz
    T2CONbits.TCS = 0;
    T2CONbits.TCKPS = 0;    // 1:1 prescale; 
    TMR2 = 0;
    OC2CONbits.OCTSEL = 0;  // Select Timer 2 as source
    OC2CONbits.OCM = 0b110; // PWM mode
    OC2RS = 2000;           // Write duty cycle to OC2R
    T2CONbits.TON = 1;
}

void InitTimer3() {

    //interni Fcy (10 MHz)
    T3CONbits.TCS = 0;

    T3CONbits.TCKPS = 2; // Preskaler 1:64 daje korak brojanja 0.0000064 sekundi

    //Ukljuci tajmer
    T3CONbits.TON = 1;
}

void Init_IC() {

    //Postavi triggere na output i stavi na 0
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD2 = 0;

    LATDbits.LATD3 = 0;
    LATDbits.LATD2 = 0;

    //Postavi echo pinove kao input
    TRISDbits.TRISD8 = 1;
    TRISDbits.TRISD9 = 1;
    
    //Input control 1 podesavanja
    IC1CONbits.ICTMR = 0;
    
    //Hvataj i uzlaznu i silaznu ivicu
    IC1CONbits.ICM = 1;

    //Interrupt za IC
    IPC0bits.IC1IP = 1;

    IFS0bits.IC1IF = 0;
 
    IEC0bits.IC1IE = 1;
    
    
    IC2CONbits.ICTMR = 0;
    
    //Input control 2 podesavanja
    IC2CONbits.ICM = 1;

    //Interrput za IC2
    IFS0bits.IC2IF = 0;

    IEC0bits.IC2IE = 1;
}
