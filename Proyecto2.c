/*
 * File:   nuevo.c
 * Author: chris
 *
 * Created on May 22, 2023, 6:21 PM
 */
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

#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 4000000
#define PRELOAD_TMR0 162

//DECLARACION DE FUNCIONES
void setup(void);
void PWM_duty1(float duty1);
void PWM_duty2(float duty2);
long map(long x, long in_min, long in_max, long out_min, long out_max);
void writeToMemory(uint8_t data, uint8_t address);
uint8_t readFromMemory(uint8_t address);

//DECLARACION DE VARIABLES
int count1, count2, count3, count4, contador = 0;
int servo3, servo4, data_index = 1;
char servo1, servo2;
int mode = 0;
uint8_t potValue, debounce, debounce2;

void __interrupt () isr (void){     //INTERRUPCIONES

    if(INTCONbits.RBIF){        //Interrupcion del PORTB
        PORTB = PORTB;
        INTCONbits.RBIF = 0;
    }
    
    if (T0IF){                  //Interrupcion TMR0
        contador += 1;
        //PORTB = contador;
        TMR0 = PRELOAD_TMR0;    //RST TMR0
        
        if (contador == 80) {   //Ciclo total de 20ms
            contador = 0;   
            PORTD = 3;
        }
        if (contador == servo1){    //Trigger para primer PWM manual
            RD0 = 0;
        }

        if (contador == servo2){    //Trigger para segundo PWM manual
            RD1 = 0; 
        }
        
        T0IF = 0;
    }
    
    
    
    
    if (PIR1bits.RCIF) { //Interrupcion entrada EUSART
        TXREG = RCREG;

        if (RCREG == 60){           //<     NINGUN MODO
            mode = 0;
            PORTA = 0;
        }else if (RCREG == 64) {    //@     MANUAL
            mode = 1;
            PORTA = 0;
            RA4 = 1;
        } else if (RCREG == 63) {   //?     EPPROM
            mode = 2;
            PORTA = 0;
            RA5 = 1;
        } else if (RCREG == 62 || RCREG == 61) { //>     EUSART   Y ADAFRUIT
            data_index = 1;
            mode = 3;
            if (RCREG == 62) {
                PORTA = 0;
                RA6 = 1;
            } else if (RCREG == 61) {
                PORTA = 0;
                RA7 = 1;
            }
        } else{
        
        if (mode == 3){     //En modo EUSART y ADAFRUIT obtenemos 10 digitos para controlar los servos
            
        switch (data_index) {
                //GIRO
            case 1: //Digito 1
                count1 = 0;
                count1 += (RCREG - 48)*100; //Valor centenas giro
                data_index = 2;
                break;
            case 2: //Digito 2
                count1 += (RCREG - 48)*10; //Valor decenas giro
                data_index = 3;
                break;
            case 3: //Digito 3
                count1 += (RCREG - 48); //Valor unidades giro
                data_index = 4;
                servo1 = (char) map(count1, 0, 100, 7, 16);
                break;
                //BRAZO
            case 4: //Digito 1
                count2 = 0;
                count2 += (RCREG - 48)*100; //Valor centenas brazo
                data_index = 5;
                break;
            case 5: //Digito 2
                count2 += (RCREG - 48)*10; //Valor decenas brazo
                data_index = 6;
                break;
            case 6: //Digito 3
                count2 += (RCREG - 48); //Valor unidades brazo
                data_index = 7;
                servo2 = (char) map(count2, 0, 100, 7, 16);
                break;
                //ANTEBRAZO    
            case 7: //Digito 1
                count3 = 0;
                count3 += (RCREG - 48)*100; //Valor centenas antebrazo
                data_index = 8;
                break;
            case 8: //Digito 2
                count3 += (RCREG - 48)*10; //Valor decenas antebrazo
                data_index = 9;
                break;
            case 9: //Digito 3
                count3 += (RCREG - 48); //Valor unidades antebrazo
                data_index = 10;
                servo3 = map(count3, 0, 100, 25, 50);
                PWM_duty1(servo3);
                break;
            case 10:                    //Garra abierta o cerrada
                if (RCREG == 49) {
                    servo4 = 50;
                } else {
                    servo4 = 25;
                }
                PWM_duty2(servo4);
                data_index = 1;
                TXREG = 13;
                break;

        }}}
        
        
    }
}

//LOOP PRINCIPAL
void main(void) {
    setup();
    while(1){
        
        if (mode == 1){         //Encender conversion ADC si el modo es 1
            ADCON0bits.GO = 1; 
        }else{ADCON0bits.GO = 0;}
        
        if (ADIF) {             //Cambio de canales
            if (ADCON0bits.CHS == 0b0000) { //Canal AN0
                count1 = ADRESH; 
                servo1 = (char) map(count1, 0, 255, 7, 16);     //Cambia trigger del PWM manual
                __delay_ms(20);
                ADCON0bits.CHS = 0b0001; //Cambio canal
            } else if (ADCON0bits.CHS == 0b0001) { //Canal AN1
                count2 = ADRESH; 
                servo2 = (char) map(count2, 0, 255, 7, 16);
                __delay_ms(20);
                ADCON0bits.CHS = 0b0010; //Cambio canal
            } else if (ADCON0bits.CHS == 0b0010) { //Canal AN2
                count3 = ADRESH; 
                servo3 = map (count3, 0, 255, 25, 50);
                PWM_duty1(servo3);                              //Configuracion PWM
                __delay_ms(20);
                ADCON0bits.CHS = 0b0011; //Cambio canal
            } else if (ADCON0bits.CHS == 0b0011) {  //Canal AN3
                count4 = ADRESH; 
                
                if (count4 < 128){
                    servo4 = 25;
                }else{
                    servo4 = 50;
                }
                
                PWM_duty2(servo4);
                __delay_ms(20);
                ADCON0bits.CHS = 0b0000; //Cambio canal
            }

            ADIF = 0;
        }
        
     
    if (mode == 2){     //Modo EPPROM
            
        if(RB0 == 0){                           //Debouncer
            debounce = 1;
        }
        if(RB0 == 1 && debounce == 1){      //Guardar los 4 valores de los servos
            writeToMemory((uint8_t) servo1, 0x01); //Escribe el valor del potenciometro a la EEPROM
            __delay_ms(10);
            writeToMemory((uint8_t) servo2, 0x02); //Escribe el valor del potenciometro a la EEPROM
            __delay_ms(10);
            writeToMemory((uint8_t) servo3, 0x03); //Escribe el valor del potenciometro a la EEPROM
            __delay_ms(10);
            writeToMemory((uint8_t) servo4, 0x04); //Escribe el valor del potenciometro a la EEPROM
            debounce = 0;
        }

        if(RB1 == 0){                           //Debouncer
            debounce2 = 1;
        }

        if(RB1 == 1 && debounce2 == 1){     //Leer los 4 valores de servo de la memoria y configurarlos
            servo1 = (char) readFromMemory(0x01);
            __delay_ms(10);
            servo2 = (char) readFromMemory(0x02);
            __delay_ms(10);
            servo3 = (char) readFromMemory(0x03);
            __delay_ms(10);
            PWM_duty1(servo3);
            servo4 = (char) readFromMemory(0x04);
            __delay_ms(10);
            PWM_duty2(servo4);
            debounce = 0;
        }
            }  
        
    }
    return;
}

//SETUP
void setup(void){   
    
    //Leer posicion guardada en la EPPROM
    servo1 = (char) readFromMemory(0x01);
    __delay_ms(10);
    servo2 = (char) readFromMemory(0x02);
    __delay_ms(10);
    servo3 = (char) readFromMemory(0x03);
    __delay_ms(10);
    servo4 = (char) readFromMemory(0x04);
    
    // Configuracion puertos
    ANSEL = 0b00001111;
    ANSELH = 0;
    
    TRISA = 0b00001111;
    PORTA = 0;
    
    TRISB = 0;
    PORTB = 0b00000011;
    
    //Configuracion de pullups para el PORTB
    OPTION_REGbits.nRBPU = 0;
    WPUBbits.WPUB0 = 1;
    WPUBbits.WPUB1 = 1;
    
    //Configuracion de interrupciones PORTB
    INTCONbits.RBIE = 1;
    INTCONbits.RBIF = 0;
    IOCBbits.IOCB0 = 1;
    IOCBbits.IOCB1 = 1;
    
    TRISC = 0b10000000;
    PORTC = 0;
    
    TRISD = 0;
    PORTD = 0b00000011;
    
    //CONFIGURACION OSCILADOR INTERNO
    OSCCONbits.IRCF=0b110;          //4MHz oscilador interno   
    OSCCONbits.SCS=1;
    
    //Configuracion EUSART
    TXSTAbits.SYNC = 0;             //Modo asincrono
    TXSTAbits.BRGH = 1;             //High speed baud rate

    BAUDCTLbits.BRG16 = 1;          //16-bit Baud Rate
    SPBRG = 103;                     //9600 Baud Rate
    SPBRGH = 0;
    
    RCSTAbits.SPEN = 1;             //Serial port enable
    RCSTAbits.RX9 = 0;              //8 bits de datos
    RCSTAbits.CREN = 1;             //Habilitar para recibir datos
    
    TXSTAbits.TXEN = 1;             //Habilitar para enviar datos
    
    //Interrupciones EUSART
    PIR1bits.RCIF = 0;            //Bandera RX
    //PIE1bits.RCIE = 1;            //INT EUSART RC
    
    //CONFIGURACION TIMER0
    
    OPTION_REGbits.T0CS = 0;                //Configuracion TMR0
    OPTION_REGbits.PSA = 1;
    OPTION_REGbits.PS = 0b000;
    TMR0 = PRELOAD_TMR0;
    T0IF = 0;
    T0IE = 1;
    
    //CONFIGURACION TIMER2
    CCP1CON = 0b00001100;   //Encendiendo PWM1
    CCP2CON = 0b00001100;
    
    PR2 = 250;
    
    T2CON = 0;
    T2CONbits.T2CKPS1 = 1;
    T2CONbits.T2CKPS0 = 0;
    TMR2 = 0;
    
    //Configuracion inicial PWM
    PWM_duty1(servo3);
    PWM_duty2(servo4);
    
    T2CONbits.TMR2ON = 1;
    
    //Configuracion ADC
    PIR1bits.ADIF=0;            //Bandera ADC
    //PIE1bits.ADIE=1;            //INT ADC
    ADCON0bits.ADCS=0b01;       //Conversion de reloj
    ADCON0bits.CHS=0b0000;      //Seleccion Canal AN0
    ADCON1bits.ADFM=0;          //Justificado a la izquierda
    
    ADCON1bits.VCFG0=0;         //Referencias
    ADCON1bits.VCFG1=0;
    ADCON0bits.ADON=1;          //GO
    
    INTCONbits.GIE = 1;         //INT globales
    INTCONbits.PEIE= 1;          //INT perifericas
    
    ADCON0bits.GO=1;
}

void PWM_duty1(float duty1){      //Configuracion ciclo de trabajo
    
    int new_pwm_10bits_1 =  (int) (duty1 * 10);   //Calculo de nuevo valor para el ciclo de trabajo
    //EL CCPR1 completo es 1000 por la formula en el datasheet, al multiplicar un porcentaje por 10 hace
    //bien la conversion 50*10 = 500

        CCPR1L    =   new_pwm_10bits_1>>2;                    //Seleccion nuevo ciclo de trabajo
        CCP1CONbits.DC1B1 =   new_pwm_10bits_1>>1;
        CCP1CONbits.DC1B0 =   new_pwm_10bits_1>>0;
        
    return;
}

void PWM_duty2(float duty2){      //Configuracion ciclo de trabajo
    
    int new_pwm_10bits_2 =  (int) (duty2 * 10);
        
        CCPR2L    =   new_pwm_10bits_2>>2;                    //Seleccion nuevo ciclo de trabajo
        CCP2CONbits.DC2B1 =   new_pwm_10bits_2>>1;
        CCP2CONbits.DC2B0 =   new_pwm_10bits_2>>0;
    
    return;
}

//Funcion para mapear valores dentro de un rango a otro como una regla de 3
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void writeToMemory(uint8_t data, uint8_t address){
    EEADR = address;            //Direccion de memoria
    EEDAT = data;               //Dato a escribir
    
    EECON1bits.EEPGD = 0;       //Permite acceso a la memoria de datos
    EECON1bits.WREN = 1;        //Enable para escribir
    INTCONbits.GIE = 0;         //!Apagar interrupciones globales!
    
    EECON2 = 0x55;              //Secuencia obligatoria
    EECON2 = 0xAA;
    EECON1bits.WR = 1;          //Escribir
    
    INTCONbits.GIE = 1;         //Activar interrupciones de nuevo
    EECON1bits.WR = 0;          //Apagar el escribir
    
}

uint8_t readFromMemory(uint8_t address){
    EEADR = address;            //Direccion de memoria
    EECON1bits.EEPGD = 0;       //Permite acceso a la memoria de datos
    EECON1bits.RD = 1;          //Lee el dato
    return EEDAT;               //Regresa el dato de 8 bits
    
}
