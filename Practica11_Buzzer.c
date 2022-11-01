/*
 * Practica_11_-Buzzer.c
 *
 * Created: 26/10/2022 08:52:55 p. m.
 * Author : Cesar y Antonio
 */ 
///Headers
#define  F_CPU 16000000UL
#include <avr/io.h>
#include "lcd/lcd.h" //Usa el puerto C para el display
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//////////////////////////////////////////////////////////////////////////
//Keypad
char teclas[16]={'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};
char lec;
char txt[16] ,array[16], scrib[]="00";
uint8_t read;
//Boolean y Keys
bool start=false;  //Condiciona cuando  configura/empieza
bool nozeros=true; //Condicional cuando se llega a 0
uint8_t minikey=1, nanokey=1;
//Interfaz
uint8_t save[2];
uint8_t m=0, s=0, seleccion;
char segundos[10],minutos[10],horas[10];
//Pivotes
uint8_t j,i,l;
char dumi;
// Char Especial
uint8_t clk[7]={0x1F,0x11,0x0A,0x04,0x0A,0x11,0x01F};//Caracter izquierda

void ch_reloj(void){//Caracter especial de Reloj de arena
	lcd_command(64); //Espacio de memoria ch1
	for (l=0;l<7;l++){lcd_data(clk[l]);}
	lcd_gotoxy(0,0);
	lcd_data(0);
	_delay_ms(10);
}
		
void control(void){ //Función que controla el timer y el ecendido del buzzer 
	if ((start==true)){
		if((m==0)&(s==0)){//Si llega a cuenta minima (00:00)
			start=false; nozeros=false; 
			PORTL|=(1<<PL2); //Enciende Buzzer
			seleccion=4; 
		}
		if ((s<=0)&(nozeros==true)){s=59;m--;}//Decremento en minutos, seteo seg
		if (m<0){m=0;}
	}
	else{
		m=save[1];  s=save[0]; //Recupera si esta en modo configuracion
	}
	//Segundos
	if (s<=9){sprintf(segundos,"0%d",s);}
	else{sprintf(segundos,"%d",s);}
	//Minutos
	if (m<=9){sprintf(minutos,"0%d",m);}
	else{sprintf(minutos,"%d",m);}
}

void out_lcd(void){//Salida al LCD
	control(); 
	lcd_clrscr();
	lcd_home();
	ch_reloj();//Coloca el Ch_especial en (0,0)
	 lcd_gotoxy(1,0);
	switch(seleccion){//Dependiendo se switch se indica que modo esta
		case 0:  lcd_puts("Timer");		 break; //Modo Conteo/Espera
		case 1: lcd_puts("Set?->Timer"); break;//Modo configuracion
		case 2: lcd_puts("Ins.time");	 break; //Modo inicial/Condicion
		case 3: lcd_puts("OK Set->Timer"); break;//Modo aplicando ajustes
		case 4: lcd_puts("Time!-Res->B");break;//Modo Alarma
		case 5: lcd_puts("Mute Alarm");	 break;//Modo mute
		default: break;
	}
	sprintf(array,"%s:%s",minutos,segundos);
	lcd_gotoxy(0,1);
	lcd_puts(array);
}

void Int_Enable(void){
	//Timer TOV 16-bits
	TCCR1A=0x00;//Timer 1
	TCCR1B|=(1<<CS12)|(1<<CS10);//prescaler 1024 T1
	TIMSK1|=(1<<TOIE1);//Mascara TOIE1 
	//Keypad
	EICRA|=(1<<ISC01)|(1<<ISC00);
	EIMSK|=(1<<INT0);
	sei();
}

void Puertos(void){
	DDRA&=~(1<<DDA3)|~(1<<DDA2)|~(1<<DDA1)|~(1<<DDA0); //Se declara puertos de entrada para KEYBOARD ENCODER  74C922N
	PORTA&=~(1<<PA3)|~(1<<PA2)|~(1<<PA1)|~(1<<PA0); //Se colocan en bajo las entradas
	DDRL|=(1<<DDL2);	PORTL&=~(1<<PL2);  //Salida para Buzzer
	TCNT1H=0xC2; TCNT1L=0xF7; //1 sec 
}

void cursor(void){
	nanokey^=1;
	switch (j){
		case 0: if(nanokey==0){lcd_gotoxy(4,1);} else{lcd_gotoxy(3,1);} break;
		case 1: if(nanokey==0){lcd_gotoxy(1,1);} else{lcd_gotoxy(0,1);} break;
		default: if(nanokey==0){lcd_gotoxy(4,1);} else{lcd_gotoxy(3,1);} break;
	}
}

void Set(void){
	minikey^=1;//X0R
	if (minikey==1){//Modo salida de configuración
		i=0;j=0;
		lcd_init(LCD_DISP_ON);
		if((s<=59)&(m<=59)){//Solo pasa si se indico un tiempo 
		seleccion=3; out_lcd();
		_delay_ms(1000);
		seleccion=0; out_lcd();
		start=true;
		}
		else{//Retorna si no se indico un tiempo
		seleccion=2; out_lcd();
		_delay_ms(1000);
		seleccion=0; out_lcd();
		start=false;
		}
	}
	else{//Modo configuracion
		lcd_init(LCD_DISP_ON_CURSOR_BLINK);
		if((m==0)|(s==0)){ 
			seleccion=1; out_lcd();
			cursor();
			save[1]=m; save[0]=s;
			start=false;
		}
		else{
			i=0; j=0;
			lcd_init(LCD_DISP_ON);
			seleccion=1; out_lcd();
			_delay_ms(1000);
			seleccion=0; out_lcd();
			save[1]=m; save[0]=s;
			start=false;
		}
	}
}

void Mov(void){//Se mueve para los minutos o segundos con la tecla C
	if (j<=1){j++; cursor(); }
	else{j=0; cursor();}
	scrib[0]=scrib[1]='0';
	i=0;
}

void Mute(){//Apaga el buzzer con la tecla B
	PORTL&=~(1<<PL2);//OFF
	nozeros=true;//Se regresa condiconales a estado inicial
	start=false;
	seleccion=5; out_lcd();
	_delay_ms(1000);
	seleccion=0; out_lcd();
}

void Push(void){//Funcion de ingreso de datos (inspirado en app de reloj de windows)
	dumi=lec;
	scrib[0]=scrib[1]; 
	scrib[1]=dumi;	//Desplazamiento 
	switch(j){
		case 0: save[0]=s=atoi(scrib); break;//Segundos
		case 1: save[1]=m=atoi(scrib); break;//Minutos
		default: j=0; break;
	}
	if (s>59){//Filtro valor maximo segundos
		scrib[0]='0';
		scrib[1]=dumi; 
		save[0]=s=atoi(scrib); i=0; 
	}
	if (m>59){//Filtro valor maximo minutos
		scrib[0]='0';
		scrib[1]=dumi;
		save[1]=m=atoi(scrib); i=0;
	}
}

void Input(void){//Funcion general para valor numerico de teclado
	if(i<2){
		Push();
		out_lcd();
		i++; 
	}
	else{
		Push();
		out_lcd();
		i=0;
	}
	cursor();
}

void keypad(void){ //Funcion general o especifica para cada boton 
	lec=teclas[read];
	switch(lec){
		case 'A': if(nozeros==true){Set();} break;		//Parar y arrancar
		case 'C': if(start==false){ Mov();  } break;	//Moverse entre minutos y segundos
		case 'B': if(nozeros==false) { Mute(); } break;	//Silenciar el Buzzer
		default:  if(start==false){if((lec!='*')&(lec!='#')&(lec!='D')){ Input();}} break;	//Regupera digitos pulsados
	}
}

ISR(INT0_vect){
	read=0x0F&PINA; //Lectura del puerto con lo que entrga el encoder
	keypad();
}

ISR(TIMER1_OVF_vect){//Contador de 1 segundo
	if ((start==true)&(nozeros==true)){//Solo disminuye si se inicia el timer, o no se llega a la cuenta minima
		s--; 
	    out_lcd();
	}
	TCNT1H=0xC2; TCNT1L=0xF7;
}

int main(void)
{
	Int_Enable();
	Puertos();
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_home();
	out_lcd();
	Set();
    while (1) 
    {
    }
}

