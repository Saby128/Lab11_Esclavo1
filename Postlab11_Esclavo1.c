/* 
 * File:   Esclavo.c
 * Author:Saby Andrade
 *
 */

// Configuracion1
#pragma config FOSC = INTRC_NOCLKOUT    // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF               // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF              // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF              // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF                 // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF                // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF              // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF               // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF              // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF                // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// Configuracion 2
#pragma config BOR4V = BOR40V         
#pragma config WRT = OFF               

/* 
 *Librerías
 */
#include <xc.h>
#include <stdint.h>


/* 
 *Constantes
 */
#define _XTAL_FREQ 1000000      
#define POT_MIN 0          
#define POT_MAX 253              
#define PWM_MIN 60            
#define PWM_MAX 123            

/* 
 *Variables
 */
uint8_t Signal_PWM;              
uint8_t Datos;               

/* 
 *Prototipo de Funciones
 */
void setup(void);

unsigned short arreglo(uint8_t valor, uint8_t minimo_potenciometro, uint8_t maximo_potenciometro, 
            unsigned short minimo_PWM, unsigned short maximo_PWM);


/* 
 *Interrupciones
 */
void __interrupt() isr (void)
{    
    if (PIR1bits.SSPIF)
    {              
        Datos = SSPBUF;             
        Signal_PWM = arreglo(Datos, POT_MIN, POT_MAX, PWM_MIN, PWM_MAX);   
        CCPR1L = (uint8_t)(Signal_PWM>>2);        
        CCP1CONbits.DC1B = Signal_PWM & 0b11;    
        PIR1bits.SSPIF = 0;           
    }
    return;
}

/* 
 *Ciclo Principal
 */
void main(void) 
{
    setup();
    while(1)
    {        
    }
    return;
}

/* 
 *Configuracion
 */
void setup(void)
{   
    TRISC = 0b00111000;       
    PORTCbits.RC5 = 0;
       
    OSCCONbits.IRCF = 0b100;    // 1MHz
    OSCCONbits.SCS = 1;         // Reloj interno

    ANSEL = 0;
    ANSELH = 0;              // I/O digitales
    TRISA = 0b00100000;         // SS como entrada
    TRISD = 0;               // PORTD como salida
  
    PORTA = 0;               // Limpieza del PORTA
    PORTC = 0;               // Limpieza del PORTC
    PORTD = 0;               // Limpieza del PORTD

    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de perifericos
    PIR1bits.SSPIF = 0;         
    PIE1bits.SSPIE = 1;         // Habilitar interrupciones de SPI

    SSPCONbits.SSPM = 0b0100;   // SPI Esclavo, SS hablitado
    SSPCONbits.CKP = 0;         // Reloj inactivo en 0
    SSPCONbits.SSPEN = 1;       // Habilitamos pines de SPI
    SSPSTATbits.CKE = 1;        // Dato enviado cada flanco de subida
    SSPSTATbits.SMP = 0;      

    PIR1bits.TMR2IF = 0;        // Limpiamos bandera de interrupcion del TMR2
    T2CONbits.T2CKPS = 0b11;    // prescaler 1:16
    T2CONbits.TMR2ON = 1;       // Encendemos TMR2
    while(!PIR1bits.TMR2IF);    // Esperar un cliclo del TMR2
    PIR1bits.TMR2IF = 0;        // Limpiamos bandera de interrupcion del TMR2 nuevamente
    
    TRISCbits.TRISC2 = 1;       // Deshabilitamos salida de CCP1
    PR2 = 61;                   // periodo de 4 ms

    CCP1CON = 0;                // Apagamos CCP1
    CCP1CONbits.P1M = 0;        // Modo single output
    CCP1CONbits.CCP1M = 0b1100; // PWM       
    CCPR1L = 250>>2;
    CCP1CONbits.DC1B = 250 & 0b11;    
   
    TRISCbits.TRISC2 = 0;       
}

unsigned short arreglo(uint8_t a, uint8_t a0, uint8_t a1, 
            unsigned short b0, unsigned short b1)
{
    return (unsigned short)(b0+((float)(b1-b0)/(a1-a0))*(a-a0));
}