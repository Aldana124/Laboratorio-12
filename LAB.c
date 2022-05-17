/* 
 * File:   LAB.c
 * Author: diego
 *
 * Created on 16 de mayo de 2022, 06:56 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 1000000 //Frecuencia de Oscilador

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
uint8_t POT = 0;    //Variable para lectura pot
uint8_t SWITCH = 0; //Bandera de sleep
uint8_t ADDRESS = 0;//Dirección de datos
/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);
uint8_t CARGAR(uint8_t ADDRESS);
void GUARDAR(uint8_t ADDRESS, uint8_t VALORES);

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.ADIF){      //Interrupción ADC             
        POT = ADRESH;       //Carga lectura ADC en variable POT            
        PORTD = POT;        //Se muestra POT en PORTD         
        PIR1bits.ADIF = 0;  //Limpieza bandera        
    }
    else if(INTCONbits.RBIF){  //Interrupción PORTB        
        if(!PORTBbits.RB0){    //Alteración en RB0                 
            if(SWITCH==1){     //Bandera de sleep 1
                SWITCH = 0;    //Bandera de sleep 0
            }
            else{     
                SWITCH = 1;}  //Bandera de sleep 1
            }
        if(!PORTBbits.RB1){    //Alteración en RB1     
           GUARDAR (ADDRESS, POT);  //Se cargan los valores a memoria
        }
        
        INTCONbits.RBIF = 0; //Limpieza bandera      
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();
    while(1){ 
        if(ADCON0bits.GO == 0){ // Si no hay conversión...
            ADCON0bits.GO = 1;  // se inicia la conversión     
        }
        if(SWITCH == 1){         //Modo sleep     
            PIE1bits.ADIE = 0;
            SLEEP();                
        }
        else if(SWITCH == 0){    //Modo ON          
            PIE1bits.ADIE = 1;
        }
        PORTC = CARGAR(ADDRESS);  //Se cargan los valores de dirección de memoria al PORTC
        __delay_ms(500);
    }  
    return;
}


uint8_t CARGAR(uint8_t ADDRESS){ //Cargar datos a la memoria
    EEADR = ADDRESS;             
    EECON1bits.EEPGD = 0;  
    EECON1bits.RD = 1;      
    return EEDAT;              
}

void GUARDAR(uint8_t ADDRESS, uint8_t VALORES){ //Leer datos de memoria
    EEADR = ADDRESS;
    EEDAT = VALORES;
    EECON1bits.EEPGD = 0; 
    EECON1bits.WREN = 1;  
    INTCONbits.GIE = 0;    
    EECON2 = 0x55;      
    EECON2 = 0xAA;
    EECON1bits.WR = 1;   
    EECON1bits.WREN = 0;  
    INTCONbits.RBIF = 0;
    INTCONbits.GIE = 1; 
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    //Configuración Entradas/Salidas
    ANSEL = 0b00000001; //Analogica AN0
    ANSELH = 0; // I/0 Digitales
    TRISB = 0b00000011;; //Entradas PORTB
    PORTB = 0; //Limpieza PORTB
    TRISA = 0b00000001; //Entrada PORTA
    TRISC = 0; //Salida PORTC
    TRISD = 0; //Salida PORTD
    PORTC = 0; //Limpieza PORTC
    PORTD = 0; //Limpieza PORTD
    PORTA = 0; //Limpieza PORTA
   
    //Configuración PORTB
    OPTION_REGbits.nRBPU = 0;
    WPUB = 0b11;   
    IOCB = 0b11;   
    INTCONbits.RBIE = 1;  //Habilitación de bandera PORTB
    INTCONbits.RBIF = 0;  //Limpieza de bandera
    
    //Configuración ADC
    ADCON0bits.ADCS = 0b00;     // Fosc/2
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0;         // Seleccionamos el AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitamos modulo ADC
    __delay_us(40);             // Sample time

    //Configuración Interrupciones
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;        // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;         // Habilitamos int. globales
     

}