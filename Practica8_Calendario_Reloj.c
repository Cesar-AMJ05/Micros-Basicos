/*
 * Práctica 8_Calendario_Reloj.c
 *
 * Created: 16/10/2022 12:09:21 p. m.
 * Author : Cesar Y Antonio
 */ 
#define  F_CPU 16000000UL //Osilador
#include <avr/io.h>			
#include "lcd/lcd.h"		//Libreria de LCD Ojo: Dentro del lcd.h se configuro las salidas del Puerto C
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//////////////////////////////////////////////////////////////////////////
//Condicionales 
bool enter=true;
bool stop=true;
//Keypad
uint8_t read,minikey=1;
char scrib[10];
char lec;
char array_calendar[16];
char array_clock[16];
//char teclas[16]={'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'}; //Funciona Unicamente en Proteus
char teclas[16]={'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};
//Clock ans Date
uint8_t s,m,h,d,mt;//Segundos-Minutos-Horas-Dias-Meses
uint16_t y;//Años
uint16_t save[5]; //j
uint8_t NumDia; //Numero de dias respecto al mes 
uint8_t ms;//Desbordes
char segundos[10],minutos[10],horas[10],dias[10],meses[10],years[10];
//Configuración
uint8_t dig=0;
//Pivotes
uint8_t i,j,l,f,g;

void Puertos(void){ //Se declaran los puertos de entrada del Keypad
	DDRA|=(0<<PINA3)|(0<<PINA2)|(0<<PINA1)|(0<<PINA0);
}

void Interrupciones(void){ //Interrupciones a utilizar
	//Keypad_
	EICRA|=(1<<ISC01)|(1<<ISC00);
	EIMSK|=(1<<INT0);				//Interrupcion externa
	//Timer
	TCCR0A=0x00;				 //Configura el Normal Mode y clear and compare match
	TCCR0B|=(1<<CS02)|(1<<CS00); //Registro control timer B con prescaler a 1024
	TCNT0=0x00;					 //Timer counter en 0x00
	TIMSK0|=(1<<TOIE0);
	sei();
}

void Bisiesto(void){ //Funcion que determina si el año es bisiesto, determina el numero maximo de dias en febrero
	 y=save[2];
	 if ((y%4==0)&((y%100!=0)|(y%400==0))){NumDia=29;}
	 else{NumDia=28;}
}

void NumDias(void){// Determina si el mes tiene 30 o 31 dias, en caso de febrero llama a Bisiesto() para determinarlo
	if((mt==1)|(mt==3)|(mt==5)|(mt==7)|(mt==8)|(mt==10)|(mt==12)){NumDia = 31;}
	else if(mt==2){Bisiesto();}
	else{NumDia=30;}
}

void mov_int(void){//Esta funcion mueve el cursor dependiendo de lo que se este editando mientras se configura
	switch(j){
		case 0: l=3; f=1; g=2; break;// Asigna las posicion en el display (x,y) y el numero maximo de desplazamientos
		case 1: l=0; f=1; g=2; break;
		case 2: l=6; f=0; g=4; break;
		case 3: l=3; f=0; g=2; break;
		case 4: l=0; f=0; g=2; break;
		default: break;
	}
	if(i==g){lcd_gotoxy(l,f);}
	else {lcd_gotoxy((l+i),f);}
}

void out_lcd(void){ //Funcion que imprime los datos en el lcd
	if (stop==true){ //Si el reloj esta en modo normal realiza el cambio independientemente
		if (s>59){s=0;m++;}
		if (m>59){m=0;h++;}
		if (h>23){h=0;d++;}
		NumDias();
		if (d>NumDia){d=1;mt++;}
		if (mt>12){mt=1;y++;}
		//if (y>99){y=0;}
	}
	else{//Si esta en modo configuracion recupera los datos ingresados en el vector save
		m=save[0]; h=save[1]; //Clock
		d=save[4]; mt=save[3]; y=save[2]; //Calendar
	}
	//Segundos
	if (s<=9){sprintf(segundos,"0%d",s);}
	else{sprintf(segundos,"%d",s);}
	//Minutos
	if (m<=9){sprintf(minutos,"0%d",m);}
	else{sprintf(minutos,"%d",m);}
	//Horas
	if (h<=9){sprintf(horas,"0%d",h);}
	else{sprintf(horas,"%d",h);}
	//Dias
	if (d<=9){sprintf(dias,"0%d",d);}
	else{sprintf(dias,"%d",d);}
	//Meses
	if (mt<=9){sprintf(meses,"0%d",mt);}
	else{sprintf(meses,"%d",mt);}
	//Años
	if (y<9){sprintf(years,"000%d",y);}
	else if (y<99){sprintf(years,"00%d",y);}
	else if (y<999){sprintf(years,"0%d",y);}
	else{sprintf(years,"%d",y);}
	//////////////////////////////////////////////////////////////////////////
	lcd_clrscr();
	lcd_home();
	sprintf(array_calendar,"%s/%s/%s",dias,meses,years);//Fecha
	lcd_gotoxy(0,0);
	lcd_puts(array_calendar);
	sprintf(array_clock,"%s:%s:%s",horas,minutos,segundos);//Reloj
	lcd_gotoxy(0,1);
	lcd_puts(array_clock);
	if (stop!=true){mov_int();}//Solo se activa si se esta configurando 
}

void Filtro(void){//Verifica que el valor que se ingreso (dependiendo del caso) no supere el valor maximo
	//Si no se cumple se coloca un boleano en false y no permite el paso al siguiente valor
	switch(j){
		case 0: if(m>59){enter=false;} else{enter=true;} break; //Minutos
	    case 1: if(h>23){enter=false;} else{enter=true;} break;//Horas
		case 3: if(mt>12){enter=false;} else{enter=true;} break;//Meses 
		case 4: NumDias(); if(d>NumDia){enter=false;} else{enter=true;} break;//Dias
		default: enter=true;  break;	
	}
}

void Configuracion(void){//Guarda el valor ingresado por el usuario
	if (i<g){
		scrib[i]=lec;//Se recupera el valor ingresado
		save[j]=atoi(scrib);//Dependiendo de la posicion lo convierte en int y guarda en el vector save
		i++;
		out_lcd();//Se imprime
		Filtro();//Se verfica los flitros
	}
}

void caso1(void){// Si se presiona "#" y se cumple que este en modo configuracion y que el dato anterior cumple con los valores maximos
	//Avanza en el display para ir cambiando el valor requerido
	if ((stop==false) & (enter==true)){
		if (j<4){j++;}
		else{j=0;}
		i=0;
		mov_int();
		memset(scrib,'\0',(sizeof(scrib)));	//Borra el vector que guarda el valor ingresado
	}
}

void caso2(void){//Habilita y deshabilita el modo configuracion
	minikey^=1; 
	if(minikey==0){//Se usa un XOR para cambiar el estado de stop y asi activar o desactivar el reloj
		stop=false;
		lcd_init(LCD_DISP_ON_CURSOR_BLINK);//Se activa el cursor
		save[0]=m; save[1]=h; //Clock
		save[4]=d; save[3]=mt; save[2]=y; //Calendar		
		out_lcd();
		mov_int();
		}
	else{
		stop=true; ms=0; s=0; j=0; i=0; //Se resetea el timer y pivotes
		lcd_init(LCD_DISP_ON); //Se apaga el cursor
		out_lcd();
	}
}

void caso3(void){//Borra el dato ingresado previamente, esto si se ingreso un dato incorrecto
	memset(scrib,'\0',(sizeof(scrib)));
	save[j]=atoi(scrib);
	out_lcd();
	i=0;
}

void caso4(void){//Si esta en modo configuracion se bloquea los botones que no se ocupan del teclado
	// Y se manda a llamar la funcion configuracion
	if (stop==false) {
	if((lec!='B')|(lec!='C')|(lec!='D')){ Configuracion();}
	}
}

void keypad(void) {
	lec=teclas[read];//Se guarda el valor que se ingreso en el teclado
	 switch(lec){
		 case '#': caso1(); break;
		 case '*': caso2(); break;
		 case 'A': caso3(); break;
		 default: caso4();  break;
	}
	lec='\0';
}

ISR(INT0_vect){//Mediante interrupciones se activa el Encoder MMC74C9922 y se recupera el dato en binario que
	//Indica que tecla se preciono, este se compara con un arreglo que guarda mediante posicion su valor
	read=0x0F&PINA;
	keypad();
}

ISR(TIMER0_OVF_vect){//Timer que en el modo normal hace avanzar el reloj 
	ms++;
	if (ms>61) {//Si desborda  61 veces cuenta un segundo
		 ms=0; if(stop==true) {s++; out_lcd();}//Si se activa el modo configuracion no permite que sigan aumentando los segundos
	 }
}

int main(void)
{	
	Puertos(); //Se habilitan los puertos
	Interrupciones();//Se habilitan las interrupciones
	lcd_init(LCD_DISP_ON);//Se inica el display
	lcd_home();
	out_lcd();
    while (1) 
    {
    }
}

