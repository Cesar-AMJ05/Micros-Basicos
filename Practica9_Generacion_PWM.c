/*
 * Practica9_Generacion_PWM.c
 *
 * Created: 25/10/2022 02:35:12 p. m.
 * Author : cesar
 */ 
//////////////////////////////////////////////////////////////////////////
//Cabeceras
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>
#include "lcd/lcd.h" //Puerto C
//////////////////////////////////////////////////////////////////////////
/////Variables////
//Chr Especial
uint8_t wave1[7]={0x1F,0x1F,0x18,0x1B,0x1B,0x03,0x1F};//Caracter izquierda
uint8_t wave2[7]={0x1F,0x1F,0x03,0x1B,0x1B,0x18,0x1F};//Cracter derecha
//Keypad
char array[16];
char txt[16];
uint8_t read;
char teclas[16]={'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};
char lec;
char scrib[10];
uint8_t minikey=1;
bool stop = false;	//Boleano Configuracion/Cancelar
bool enter = false; //Boleano Enter
//Configuración
uint8_t intFrec=1, i,j, back; //Guardar frecuencia //numero de desplazamientos//pivote//respaldo
uint16_t x,dumi;
//Interfaz
uint8_t selector=1;

void ch_sqw(void){//Caracter Especial
	lcd_command(64); //Espacio de memoria ch1
	for (j=0;j<7;j++){lcd_data(wave1[j]);}
	lcd_gotoxy(0,0);
	lcd_data(0);
	_delay_ms(10);
	lcd_command(72);//Espacio de memoria ch2
	for (j=0;j<7;j++){lcd_data(wave2[j]);}
	lcd_gotoxy(1,0);
	lcd_data(1);
	_delay_ms(10);	
}

void out_lcd(void){
	switch(selector){ //Se selecciona el texto dependiendo del modo
		case 1: sprintf(txt,"-> 1-30 kHz"); break;   //Modo de trabajo
		case 2: sprintf(txt,"->Set 1-30 kHz"); break;//Modo configuración
		case 3: sprintf(txt,"->X-Set"); break;		 //Modo cancelar
		case 4: sprintf(txt,"#1<=f>=30 kHz!"); break;//Modo error
		case 5: sprintf(txt,"->OK-Set"); break;		 //Modo Aceptado
		default: break;
	}
	lcd_clrscr();
	lcd_home();
	ch_sqw(); //Character especial
	lcd_gotoxy(2,0);
	lcd_puts(txt);	//Salida de selector
	lcd_gotoxy(0,1);
	sprintf(array,"f-out=%d kHz",intFrec); //Muestreo de frecuencia
	lcd_puts(array);
}

void Int_Compa(void){
	//Keypad
	EICRA|=(1<<ISC01)|(1<<ISC00);
	EIMSK|=(1<<INT0);
	//Compa 8-bits Timer 0
	TCCR0A|=(1<<WGM01)|(1<<COM0A0);//Modo comparacion y Toggle
	TCCR0B|=(1<<CS01)|(1<<CS00);//Prescaler 64
	TCNT0=0x00;
	OCR0A=124; //Predeterminado para 1KHz
	TIMSK0|=(1<<OCIE0A); //Se activa la interrupcion por CTC  T0
	sei();
}

void Puertos(void){
	DDRA&=~(1<<DDA3)|~(1<<DDA2)|~(1<<DDA1)|~(1<<DDA0); //Se declara puertos de entrada para KEYBOARD ENCODER  74C922N
	PORTA&=~(1<<PA3)|~(1<<PA2)|~(1<<PA1)|~(1<<PA0); //Se colocan en bajo las entradas
	DDRL|=(1<<DDL2);//Se declara salida para CTC T0
	PORTL|=(1<<PL2);//Se declara en alto salida CTC T0
}

ISR(TIMER0_COMPA_vect){//Comparacion Timer 0 
	PORTL^=(1<<PL2); //Cada que desborde conmuta debido a la X0R
	
}

void Configuracion(void){ //Recupera los datos ingresados
	if(i<=1){
		scrib[i]=lec;//concatena en un string los char obtenidos
		intFrec = atoi(scrib); //char a uint8
		i++;
		out_lcd();	
	}
	else{
		memset(scrib,'\0',(sizeof(scrib))); //Cambia datos a nulos en el char array
		intFrec = 1 ; //Regresa al valor predeterminado
		i=0;
		out_lcd();
	}
}

void caso1(void){
	minikey^=1;//XOR para un mismo boton
	if(minikey==0){//Entrar a Configurar
		lcd_init(LCD_DISP_ON_CURSOR_BLINK); 
		stop=true;
		back=intFrec; //Se guarda la frecuencia antes del cambio
		selector=2;
		out_lcd();
	}
	else{//Salir de configurar
		lcd_init(LCD_DISP_ON);
		stop=false;
		if (enter==false){//Cancelar
			intFrec=back; //Recupera frecuencia anterior
			memset(scrib,'\0',(sizeof(scrib)));
			selector=3;
			out_lcd();
			_delay_ms(1000);
		}
		else{
			memset(scrib,'\0',(sizeof(scrib)));
			selector=5;
			out_lcd();
			_delay_ms(1000);					
		}
		i=0;
		selector=1;
		out_lcd();
		enter=false;
	}
}

void caso2(void){
	if((stop==true)&(intFrec>=1)&(intFrec<=30)){//Verifica que este configurando y cumpla con el intervalo 
		dumi=(16000000)/(intFrec*1000); //Realiza por partes la ecuacion 
		x=(dumi/(2*64))-1;
		OCR0A=(uint8_t)x;//Pasa de 16 a 8-bits Y modifica la frecuencia
		enter=true;
		stop=false;
		caso1();
	}
	else if(stop==true){//No cumple con el intervalo
		memset(scrib,'\0',(sizeof(scrib)));
		i=0;
		selector=4;
		out_lcd();
		_delay_ms(1000);
		selector=2;
		out_lcd();
	}
}

void caso3(void){
	if(stop==true){
		if((lec!='A')|(lec!='B')|(lec!='C')|(lec!='D')){Configuracion();}//Evita caracteres no utilizados en el teclado 
	}
}

void keypad(void){
	lec=teclas[read];
	switch(lec){
		case '*': caso1(); break;//Configurar o Cancelar
		case '#': caso2(); break;//Filtro y seteo de nueva frecuencia
		default: caso3(); break;//Regupera digitos pulsados
	}
}

ISR(INT0_vect){//Interrupcion que se activa cuando se acciona algun boton del teclado
		read=0x0F&PINA;//Lee del puerto C 
		keypad();
}

int main(void){
	lcd_init(LCD_DISP_ON);//Inicia el Display
	out_lcd();
	Puertos();
	Int_Compa();
    while (1) 
    {
    }
}

