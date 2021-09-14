/*
 * File:   main.c
 * Author: Jhennifer Matias (18103346)
 *
 * Created on 10 de Setembro de 2021, 14:42
 */

#include <xc.h>          //***inclus?o da biblioteca do compilador
#include <pic16f877a.h>  //***inclus?o da biblioteca do chip espec?fico
#include <stdio.h>       //***inclus?o da biblioteca standard padr?o "C"

//****** congiguration bits  **************************************************
#pragma config WDTE = OFF   //Desabilita o uso do WDT
#pragma config FOSC = HS     //define uso do clock externo EM 4 OU 20mHZ
//se usar o XT no m?ximo 4Mhz
#pragma config PWRTE = ON   //habilita reset ao ligar
#pragma config BOREN = ON   //Habilita o reset por Brown-out 

#define _XTAL_FREQ 4000000 //***Defini a frequencia do clock, 4Mhz neste caso

//botoes
#define EMERGENCIA  PORTBbits.RB0  
#define LIGAR       PORTBbits.RB1
#define UP          PORTBbits.RB3
#define DOWN        PORTBbits.RB4
#define ENTER       PORTBbits.RB5
//leds
#define ligado      PORTBbits.RB6  
#define led_emerg   PORTBbits.RB7
#define led_tempe   PORTCbits.RC5
//motor
#define motor_enable PORTCbits.RC1          //Habilita motor
#define motor1       PORTCbits.RC2          //motor1 e motor2 definem o sentido de rotação
#define motor2       PORTCbits.RC3

//*** define pinos referentes a interface com LCD
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

int celsius = 0;
int contador_minutos = 0;
int contador_horas = 0;
char timer[6];
int conta = 0;
#include "lcd.h"

//*** subrotina para tratar a interrrup??o *************************************

void mostra_timer(){
    if(contador_minutos>=60){
        contador_minutos = contador_minutos - 60;
        contador_horas++;
    }else{
        if(contador_minutos <= 0){
            if(contador_horas <= 0){
                contador_minutos = 0;
                contador_horas = 0;
            }else{
                contador_horas--;
                contador_minutos = 59;
            }
        }
    }
    
    Lcd_Clear(); //limpa LCD
    Lcd_Set_Cursor(1, 1); //P?e curso linha 1 coluna 1
    sprintf(timer, "%d:%d", contador_horas, contador_minutos);
    Lcd_Write_String(timer); //escreve string
}


void __interrupt() INTR(void){
    if(INTF)
    {
        INTF = 0;
        led_emerg = 1;
        ligado = 0;
        motor_enable = 0;
        Lcd_Set_Cursor(1, 1); //P?e curso linha 1 coluna 1
        Lcd_Write_String("EMERGENCIA!"); //escreve string
    }
    
      if (TMR1IF)  //foi a interrup??o de estouro do timer1 que chamou a int?
     {  
        PIR1bits.TMR1IF = 0; //reseta o flag da interrup??o
        TMR1L = 0xDC;        //reinicia contagem com 3036
        TMR1H = 0x0B;        
        
        //comandos pra tratar a interrup??o
        conta++;
        if (conta==1){
            contador_minutos--;
            conta = 0;
            mostra_timer();
        }
        
        if(contador_minutos == 0 && contador_horas == 0){
            motor_enable = 0;
             __delay_ms(50);
            ligado = 0;
            Lcd_Set_Cursor(1, 1); //P?e curso linha 1 coluna 1
            Lcd_Write_String("PRONTO P/ RETIRAR!"); //escreve string
        }
    }
    return;
}
     


void regula_timer(){
    while(1){
        if(ENTER == 0 && (contador_minutos != 0 || contador_horas !=0)){
           return;
        }else{
            if(UP == 0){
                contador_minutos++;
                mostra_timer();
                __delay_ms(50);
            }
        
        
            if(DOWN == 0){
                contador_minutos--;
                mostra_timer();
                __delay_ms(50);
            }  
        }   
    }
    
}

void main(void) 
{  
   //**inicializa??es************************************************
   TRISA = 0b11111111;       //configura pinos de entrada(1)e sa?da (0)
   TRISB = 0b00111111;       //configura pinos de entrada(1)e sa?da (0)
   TRISC = 0b00000000;       //configura pinos de entrada(1)e sa?da (0)
   TRISD = 0b00000000;       //configura pinos de entrada(1)e sa?da (0)
   
   //define led desligado
   ligado = 0;
   led_emerg = 0;
   led_tempe = 0;
   
   OPTION_REGbits.nRBPU = 0; //Ativa resistores de pull-ups
   
   //** configurando interrup??es ***********************************
   OPTION_REGbits.INTEDG = 1;
   INTCONbits.INTE = 1;
   
   //** configurando interrup??es ***********************************
   INTCONbits.GIE=1;       //Habiliita a int global
   INTCONbits.PEIE = 1;    //Habilita a int dos perif?ricos
   PIE1bits.TMR1IE = 1;    //Habilita int do timer 1
   
   //*** configura o timer 1 *****************************************
   T1CONbits.TMR1CS = 0;   //Define timer 1 como temporizador (Fosc/4)
   T1CONbits.T1CKPS0 = 1;  //bit pra configurar pre-escaler, nesta caso 1:8
   T1CONbits.T1CKPS1 = 1;  //bit pra configurar pre-escaler, nesta caso 1:8
        
   TMR1L = 0xDC;          //carga do valor inicial no contador (65536-62500)
   TMR1H = 0x0B;          //3036. Quando estourar contou 62500, passou 0,5s   
   
    //ficar? somente com AN0 como entrada analogica.
   ADCON1bits.PCFG0   = 0;  //configura as entradas analogicas
   ADCON1bits.PCFG1   = 1;  //configura as entradas analogicas
   ADCON1bits.PCFG2   = 1;  //configura as entradas analogicas
   ADCON1bits.PCFG3   = 1;  //configura as entradas analogicas   
   //define o clock de convers?o
   ADCON0bits.ADCS0 = 0  ;   //confirmando default Fosc/2
   ADCON0bits.ADCS1 = 0  ;   //confirmando default Fosc/2   
   ADCON1bits.ADFM = 0   ;   //Default e 0. Pra mostra que pega os dados em 8 bits 
                             //no ADRESH, pois esta justifica a esquerda
                             //Passando pra 1 pode pegar os dados em 10 bits 
                             //usando os dois registradores ADRESHL justificado a
                             //direita
   //inicializa registradores do AD
   ADRESL = 0x00;          //inicializar valor anal?gico com 0
   ADRESH = 0x00;
   ADCON0bits.ADON = 1;     //Liga AD
   
   __delay_ms(20);
   Lcd_Init();    //necess?rio para o LCD iniciar
   Lcd_Clear(); //limpa LCD
   
   //definir interrupcao e timer 
   
   Lcd_Set_Cursor(1, 1); //P?e curso linha 1 coluna 1
   Lcd_Write_String("00:00"); //escreve string
   regula_timer();
   while (1){
        ADCON0bits.CHS0 = 0; //configura canal 0 como entrada analogica
     ADCON0bits.CHS1 = 0; //configura canal 0 como entrada analogica
     ADCON0bits.CHS2 = 0; //configura canal 0 como entrada analogica
     ADCON0bits.GO = 1;   //converte
     __delay_us(10);      //tempo de conversao
     celsius = ADRESH*0x02;

        
        if(LIGAR == 0){
                ligado = 1;
                T1CONbits.TMR1ON = 1;
                motor_enable = 1; //Habilita motor
                motor1 = 1;
                motor2 = 0;

            
          
            if(celsius >= 98){
                led_tempe = 1;
                __delay_ms(300);
            }else{
                led_tempe = 0;
            }   
        }
     
    }    
}