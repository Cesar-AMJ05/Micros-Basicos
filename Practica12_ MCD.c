/*
 * Practica12_ MCD.c
 *
 * Created: 31/10/2022 08:32:11 p. m.
 * Author : Cesar y Antonio
 */ 
#define  F_CPU 16000000UL
#include <avr/io.h>
#include "lcd/lcd.h" //PORTC
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>
//////////////////////////////////////////////////////////////////////////
//Keypad
	char array[16];
	uint8_t read;
	//char teclas[16]={'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'}; //Funciona Unicamente en Proteus
	char teclas[16]={'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};
	char lec;
	char scrib[10];
//Phase Correct PWM
	uint8_t new_pwm; //Variable donde guarda el nuevo Ciclo de trabajo
//Condicionales
	bool okset=false;	//Solo permite setear el nuevo ciclo de trabajo si cumple el condicional
	bool start=true;	//Permite activar el motor y el led correspondiente al led
	bool ledR=false;	//Inicia el led Rojo a 1 seg con el Timer 5
	bool ledA=false;	//Inicia el led Azul a 0.5 seg con el Timer 5
//LCD
	uint8_t txt;	//Selecciona el texto que aparece segun el caso
//Pivotes
	uint8_t i;

//////////////////////////////////////////////////////////////////////////

void Int_PWM_EXT(void){
	//Timer 4 Fase CORRECTA Modo ICR4 en 25kHz
	TCCR4A|=(1<<WGM41)|(1<<COM4A1);		//Modo Fase Correcta ICR4 y el OCR4A
	TCCR4B|=(1<<WGM43)|(1<<CS41);		//Prescaler 8-bits
	TCCR4C=0x00;						//No se utiliza
	TCNT4H=0x00; TCNT4L=0x00;		
	ICR4=40;							// f_pwm= fosc/(2*N*ICR4) -> ICR4=16MHz/(2*8*25kHz)=40
	//Timer 5
	TCCR5A=0x00;//Modo Normal			
	TCCR5B|=(1<<CS52)|(1<<CS50);		//Pre 1024
	TCCR5C=0x00;
	TIMSK5|=(1<<TOIE5);					//Interrupcion 5
	TCNT5H=0xC2; TCNT5L=0xF7;			//1 segundo
	//TCNT1H=0xE1; TCNT1L=0x7B			//0.5 segundo
	//Keypad
	EICRA|=(1<<ISC01)|(1<<ISC00)|(1<<ISC11);
	EIMSK|=(1<<INT0)|(1<<INT1);//Keypad INT 0// Emergency button INT1
	sei();	
}

void Puertos(void){
	DDRH|=(1<<DDH3);//OC4A
	PORTH&=~(1<<PH3);
	DDRL=0x1F;// PL0-LED_R // PL1-LED_A // PL2-ENA // PL3-IN1// PL4-IN2
	PORTL=0x00;
	DDRA&=~(1<<DDA3)|~(1<<DDA2)|~(1<<DDA1)|~(1<<DDA0); //Entradas Keypad
	PORTA&=~(1<<PA3)|~(1<<PA2)|~(1<<PA1)|~(1<<PA0); 
}

void out_lcd(void){//Muestra las acciones en el lcd
	lcd_clrscr();
	lcd_home();
	switch(txt){
		case 1: lcd_puts("Set (0-100)");		break;
		case 2: lcd_puts("Save D.Cycle");		break;	
		case 4: lcd_puts("Right Turn");			break;
		case 5: lcd_puts("Left Turn");			break;
		case 7: lcd_puts("Stop Motor");			break;
		case 8:	lcd_puts("Press C-Help");		break;
		default: break;
	}
	sprintf(array,"DC=%d%%",new_pwm);
	lcd_gotoxy(0,1);
	lcd_puts(array);
}

void stop_motor(void){//Detiene el motor
	okset=false; start=false;
	PORTL=0x00;
}

void Clear(void){
	i=0; new_pwm=0;
	memset(scrib,'\0',(sizeof(scrib)));
}

void Right(void){ //Set-Right
	if ((okset==true)&(start=true)){
		TCNT4H=0x00; TCNT4L=0x00;			//Reinicia el contador PWM
		PORTL&=(0<<PL0)|(0<<PL4);			// 0-Logico para giro a la derecha
		PORTL|=(1<<PL1)|(1<<PL2)|(1<<PL3);// LED_A // ENA // Right
		ledA=true; ledR=false;					
		TCNT5H=0xC2; TCNT5L=0xF7;		//1seg
		txt=4;
		out_lcd();
	}
}

void Left(void){ //Set-Left

	if ((okset==true)&(start=true)){
		TCNT4H=0x00; TCNT4L=0x00;			//Reinicia el contador PWM
		PORTL&=(0<<PL1)|(0<<PL3);			// 0-Logico para giro a la izquierda
		PORTL|=(1<<PL0)|(1<<PL2)|(1<<PL4);// LED_R // ENA // Left
		ledR=true; ledA=false;
		TCNT1H=0xE1; TCNT1L=0x7B;		//0.5 seg
		txt=5;
		out_lcd();
	}
}

void Set(void){//Configura el ciclo de trabajo
	txt=1;
	okset=false;
	start=false;
	stop_motor();			//Detiene el motor para configurar
	Clear();
	out_lcd();
}

void Enter(void){//Setea el nuevo Ciclo de trabajo
	okset=true; start=true; //Permite iniciar los giros
	OCR4A=(new_pwm*40)/(100); //Propocionalidad con el valor maximo del OCR4A
	txt=2;
	out_lcd();
	i=0;
	memset(scrib,'\0',(sizeof(scrib)));
}

void Input(void){//Recupera datos ingrsados del keypad y se convietren e int de 8 bits
	scrib[i]=lec;
	new_pwm=atoi(scrib);
	i++;
	if (new_pwm>100){Clear();}
	out_lcd();
}

void Help(void){ //Manual Ayuda Usuario
	lcd_clrscr();
	lcd_home();
	lcd_puts("A-Right B-Left");
	lcd_gotoxy(0,1);
	lcd_puts("*-Set #-Enter");
	_delay_ms(1000);
	out_lcd();
}

void keypad(void){//Casos posibles con el Keypad
	lec=teclas[read];
	switch(lec){
		case 'A': if(okset==true){Right();}; break; 
		case 'B': if(okset==true){Left();} break; 
		case '*': Set(); break; 
		case '#': if(okset==false){Enter();} break;
		case 'C': Help(); break; 
		default:if(lec!='D'){Input();} break; //Input
	}
}

ISR(TIMER5_OVF_vect){//Int. para los leds 
	if (start==true){
		if(ledA==true){ PORTL^=(1<<PL1);TCNT5H=0xC2; TCNT5L=0xF7;}//1 segundo
		else if(ledR==true){ PORTL^=(1<<PL0);TCNT5H=0xE1; TCNT5L=0x7B;}//0.5 segundo
	}
}

ISR(INT0_vect){//Int. para el teclado
	read=0x0F&PINA;
	keypad();
}

ISR(INT1_vect){//Int. Boton de emergencia
	stop_motor();//Detiene el motor
	txt=7;
	out_lcd();
	_delay_ms(1000);
	Set();
}

int main(void){
	Int_PWM_EXT();
    Puertos();
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_home();
	txt=8;
	Help();			//Se muestra la ayuda
	_delay_ms(1000);
	Set();			//Se inicia el set
	OCR4A=0;
    while (1) 
    {
    }
}

