/*
 * Reloj_06.c
 *
 * Created: 20/09/2022 05:32:14 p. m.
 * Author : Cesar y Antonio
 */ 
#define F_CPU 12000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>

uint8_t ms=0;
uint8_t h,m,s,i,temp=0;
uint8_t seg7c[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
uint8_t hora[8]={0,0,0,0,0,0,0,0}; // dh-uh-dm-um-ds-du-ch1-ch2   Declaramos el vector que contiene las horas, minutos y segundos
uint8_t salidas[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
uint8_t contadorS = 0;
uint8_t APM[3] = {0x67, 0x77, 0x76}; // P, A, M

ISR(TIMER0_COMPA_vect) //Configuramos nuestra interrupción por tiempo 
{
		ms++;
		if (ms>50) //Despues de 50 ciclos de 20 ms obtenemos un segundo, 
		{		   //cada segundo nuestra interrupción se activara
			ms=0;
			s++;
		}
}
//Función de Multiflexado, imprime constante los valores en los leds
void Display(void)
{
		for (i=0;i<8;i++)	
		{
			PORTC=salidas[i];
			if(i==0)
			{
				PORTF=0x55;//76
			}
			else if (i==1)
			{
				if((h>=0)&(h<=11))
				{
					PORTF=0x77;
				}
				else{PORTF=0X73;}
				
			}
			else
			{
			PORTF=seg7c[(hora[i])];
			}
			_delay_ms(3);
		}	
		
}
//Función que detecta un nivel bajo en el boton conectado al puerto H3, modificando los minutos
void bttm(void)
{
	if((PINH &(1<<PINH3))==0)
	{
		while((PINH &(1<<PINH3))==0)
		_delay_ms(50);
		if(m<60)
		{
		m++;	
		}
		else{m=0;}
		hora[5]=m/10;
		hora[4]=m%10;
		Display();
	}
}
//Función que detecta un nivel alto en el boton conectado al puerto H4, modificando las horas
void btth(void)
{
	if((PINH &(1<<PINH4))!=0)
	{
		while((PINH &(1<<PINH4))!=0)
		_delay_ms(50);
		if(h<23)
		{
			h++;
		}
		else{h=0;}
		hora[7]=h/10;
		hora[6]=h%10;
		Display();
	}
}
//Boton de parado, apaga las interrupciones
void btt_stop(void)
{
	if ((PINH &(1<<PINH5))!=0)
	{
		_delay_ms(50);
		//while((PINH &(1<<PINH5))!=0)
		cli();
		while(1)
		{
			Display();
			bttm();
			btth();
			//Boton de arranque, reactiva las interrupciones
			if ((PINH &(1<<PINH6))==0)
			{
				_delay_ms(50);
				//while((PINH &(1<<PINH5))==0)
				sei();
				s=0;	
				break;
			}		
		}
	}
}

int main(void)
{
	DDRC=0xff; PORTC=0x00;								//Puerto C como salida para transistores
	DDRF=0xff; PORTF=0x00;								//Puerto F como salida para display
	DDRH|=(0<<PINH3)|(0<<PINH4)|(0<<PINH5)|(0<<PINH6);	//PIN H3-H4-H5 como entradas
	PORTH|=(0<<PINH3)|(0<<PINH4)|(0<<PINH5)|(0<<PINH6);	
	//Interrupcion por tiempo
	//DDRB=0x01;
	TCCR0A=(1<<WGM01);			//Configura el Normal Mode y clear and compare match
	TCCR0B=(1<<CS02)|(1<<CS00); //Registro control timer B con prescaler a 1024
	TIMSK0=(1<<OCIE0A);			//Output Comparacion A match "COMPA"
	OCR0A=234.375;
	sei();
//Calculo de las horas, minutos y segundos, para posteriormente imprimir los valores en el Display 	 
	while (1)
	{
		btt_stop();
		//Segundos
		if (s<60)
		{	
			hora[3]=s/10;
			hora[2]=s%10;
		}else{s=0;m++;}
		//Minutos	
		if (m<60)
		{
			hora[5]=m/10;
			hora[4]=m%10;
		}else{m=0;h++;}
		//Horas
		if(h<23)
		{
			hora[7]=h/10;
			hora[6]=h%10;
		}else{h=0;};
		Display();
	}
}


