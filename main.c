//AUTHOR: Grupa 7

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




// Metoda za skretanje, skrece samo desnom gusenicom kako bi povecao radijus skretanja i priblizio senzor zidu
// Nakon samog skretanja pomera se blago napred, ponovo zbog priblizavanja senzora zidu. Sto je senzor dalje od zida, akt odrzavanja distance (iz maina) moze da 
// nacini previliki ugao sa senzorom sto ce pokvariti korektna ocitavanja.
void turn_manuevre(){
    LEFT_TURN_GYRATE();
    __delay_ms(2160);
    FORWARD_MOVE();
    __delay_ms(600);
    STOP_MOVE();
    //ok
}

int main(){
    // Inicijalizacija pinova
    init_pins();
    // Inicijalizacija tajmera 3, koji je bazni tajmer za input capture 1 i 2
    InitTimer3();
    // Inicijalizacija tajmera 1, kojim se odbrojava preporucenih 60ms periode slanja triggera
    InitTimer1();
    // Inicijalizacija tajmera 2, sluzi samo za PWM, analogni izlaz od 2V za motor, kako ne bi isao prebrzo (5V)
    InitTimer2();
    // Inicijalizacija UART1 modula, vezan za bluetooth
    InitUART1();
    // Inicijalizacija Input Capture modula za senzor 1 i 2
    Init_IC();
    // Start tajmer 1
    T1CONbits.TON = 1;
    
    while(1){
        
        // Odrzavaj distancu sa zidom izmedju 4 i 6 cm
        if(distSD < 4){
            FORWARD_MOVE(); // blago micanje napred pri svakom skretanju, da ne bi bilo ostro, *ispostvalja se da je prekratko pa druga gusenica samo miruje, ali i to je u redu.
            __delay_ms(30);
            LEFT_TURN();
        }else if(distSD > 6){
            FORWARD_MOVE();
            __delay_ms(30);
            RIGHT_TURN();
            RS232_putst("Skrecem prema desno!\n\r");
        }else{
            // Ako je ili distanca u opsegu 4-6cm, idi napred
            if(distFW > 11.5){
                RS232_putst("Idem pravo!\n\r");
                FORWARD_MOVE();
            }else{
            // A ako je prepreka ispred, tj. napred distanca manja od 11.5, pokreni manevar za skretanje levo.
                RS232_putst("Skrecem levo!");
                turn_manuevre();
               
            }
        }
        
        sprintf(message, "Dist FW: %.1f | Dist SD: %.1f \n\r", distFW, distSD);
        RS232_putst(message);
    }

    return 1;
}

// Ne koristi se
void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void){
	// Clear interrupt flag
	IFS0bits.U1RXIF = 0;
}

// IC1 Interrupt se podize na svakoj ivice ECHO signala prvog senzora. Unutra se desava:
// proverava se da li je ivica uzlazna(prvi interrupt) ili silazna (drugi interrupt)
// Ako je uzlazna, zapamti trenutak (vrednost tajmera t1)
// Ako je silazna, zapamti trenutak, i oduzmi od njega prethodni trenutak uzlazne ivice. Dobijamo trajanje visokog signala echo.
// U slucaju overflowa (iako se to ne bi trebalo dogoditi, ali za velike udaljenosti merenja mozda i moze), koristi se druga formula za racunanje duzine (if t2<t1)
void __attribute__((__interrupt__, no_auto_psv)) _IC1Interrupt(void){
    if(!falling_edgeFW){
        t1 = IC1BUF;
        falling_edgeFW = 1;
    }else{
        t2 = IC1BUF;
        falling_edgeFW = 0;
        
        if(t2>t1){
            timeFW = t2 - t1;
        }else{
            timeFW = (0xFFFF - t1)+t2;
        }
        distFW = (34300 * 0.0000064 * timeFW)/2; // T3 korak brojanja je 0.0000064 sekundi, pogledati init.c. 343*100 m/s = 34300 cm/s
        
    }

    IFS0bits.IC1IF = 0;
}

// IC1 Interrupt se podize na svakoj ivice ECHO signala drugog senzora. Unutra se desava:
// proverava se da li je ivica uzlazna(prvi interrupt) ili silazna (drugi interrupt)
// Ako je uzlazna, zapamti trenutak (vrednost tajmera t1)
// Ako je silazna, zapamti trenutak, i oduzmi od njega prethodni trenutak uzlazne ivice. Dobijamo trajanje visokog signala echo.
// U slucaju overflowa (iako se to ne bi trebalo dogoditi, ali za velike udaljenosti merenja mozda i moze), koristi se druga formula za racunanje duzine (if t2<t1)
void __attribute__((__interrupt__, no_auto_psv)) _IC2Interrupt(void){
      if(!falling_edgeSD){
        t3 = IC2BUF;
        falling_edgeSD = 1;
    }else{
        t4 = IC2BUF;
        falling_edgeSD = 0;
        
        if(t4>t3){
            timeSD = t4 - t3;
        }else{
            timeSD = (0xFFFF - t3) + t4;
        }
        distSD = (34300 * 0.0000064 * timeSD)/2; // T3 korak brojanja je 0.0000064 sekundi, pogledati init.c. 343*100 m/s = 34300 cm/s
    }

    IFS0bits.IC2IF = 0;
}

// Interrupt tajmera 1 se dize svakih 60ms. U njemu se okida trigger oba senzora, u trajanju od 3*6.4us (TMR1 korak je 6.4us). 60ms po preporuci datasheeta. Trigger
// potrebno drzati visokim bar 10us, ovde je 19.2us
void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void){
    TRIG_FW = 1;
    TRIG_SD = 1;
    TMR1=0;
    while(TMR1 < 3);
    TRIG_FW = 0;
    TRIG_SD = 0;
    
	IFS0bits.T1IF = 0;   
}

// Ne koristi se
void __attribute__((__interrupt__)) _T2Interrupt(void){
   	TMR2 = 0;
    IFS0bits.T2IF = 0;
}