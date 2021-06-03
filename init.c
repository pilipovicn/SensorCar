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

    //Use internal Fcy (10 MHz)
    T3CONbits.TCS = 0;

   
    //10 counts for 1us
    T3CONbits.TCKPS = 2; // ^-- not

    //Turning it on
    T3CONbits.TON = 1;
}

void InitTimer4(){
    	TMR4 = 0;
        
	T4CONbits.TCS = 0; //source is Fcy
	T4CONbits.TCKPS = 3; //1:512
        //Set PR1 to 60ms
	//PR1 = 37500;
    PR4 = 20000;
    
    //IFS0bits.T4IF = 0;

	// Enable the interrupt for Timer 1
	//IEC0bits.T4IE = 1;
    
}

void InitHC_SR04() {

    //Pin 16 (RP7) and Pin 15 (RP6) are required for this sensor
    //because they are 5V tolerant

    //Using RP7 for trigger (output)
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD2 = 0;
    //configure for open-drain to allow 5V on pin
    //ODCBbits.ODCB7 = 1;

    //open drain pins are low when the output is set high
    LATDbits.LATD3 = 0;
    LATDbits.LATD2 = 0;



    //Using RP6 as echo (input)
    TRISDbits.TRISD8 = 1;
    TRISDbits.TRISD9 = 1;
    //Map IC1 to RP6
    //RPINR7bits.IC1R = 6;
    //Interrupt on every 2nd event
    //According to the sensor datasheet the echo pin will only go high
    //after the trigger has been set. So the 2nd event should always be
    //the falling edge
    //IC1CONbits.ICI = 1;

    
    
    //setting up timer2
    IC1CONbits.ICTMR = 0;
    
    //Capture events on rising & falling edge
    IC1CONbits.ICM = 1;

    //Setup the IC1 interrupt
    //Set priority level (need to check doc for this) This line is straight
    //from the example in ref manual
    IPC0bits.IC1IP = 1;
    //Clear IF
    IFS0bits.IC1IF = 0;
    //Enable IC1 interrupt
    IEC0bits.IC1IE = 1;
    
    
    IC2CONbits.ICTMR = 0;
    
    //Capture events on rising & falling edge
    IC2CONbits.ICM = 1;

    //Setup the IC1 interrupt
    //Set priority level (need to check doc for this) This line is straight
    //from the example in ref manual
    //IPC0bits.IC2IP = 1;
    //Clear IF
    IFS0bits.IC2IF = 0;
    //Enable IC1 interrupt
    IEC0bits.IC2IE = 1;
    

}
