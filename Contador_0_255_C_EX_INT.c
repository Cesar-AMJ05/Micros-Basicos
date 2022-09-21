/*
 * Contador en C.c
 *
 * Created: 20/09/2022 05:31:17 p. m.
 * Author : Anton
 */ 

#define F_CPU 16000000UL //Valor el valor del cristal oscilador para la funcion de delay
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

int16_t i=0;
uint8_t u,d,c,temp;
uint8_t seg7c[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};

ISR(INT0_vect){
	EIFR |= (1<<INTF0);
	i++;
	if (i>=255)
	{
		i=0;
		u=d=c=0;
	}
	else
	{
		c=i/100;		//Centenas
		temp=i%100;		//Residuo centenas
		d=temp/10;		//Decenas
		u=temp%10;		//Unidades
	}
}

ISR(INT1_vect){
	EIFR |= (1<<INTF1);
	i--;
	if (i<=0)
	{
		i=255;
		u=d=5;
		c=2;
	}
	else
	{
		c=i/100;		//Centenas
		temp=i%100;		//Residuo centenas
		d=temp/10;		//Decenas
		u=temp%10;		//Unidades
	}
}

int main(void){
	cli();		// Deshabilita las interrupciones
	DDRF|=(1<<DDF7)|(1<<DDF6)|(1<<DDF5)|(1<<DDF4)|(1<<DDF3)|(1<<DDF2)|(1<<DDF1)|(1<<DDF0);
	PORTF =0x00;
	DDRC =0xff; PORTC =0x00;
		 
EICRA |=(1<<ISC01)|(1<<ISC00);	//Interrupcion  PULL DOWN INT0
EICRA |=(1<<ISC11)|(1<<ISC10);	//Interrupcion  PULL DOWN INT1
EIMSK |=(1<<INT0)|(1<<INT1);	// INT 0 y INT1
sei();			// Habilita las interrupciones
	
	while(1)
	{
		PORTC = 0x01;			//Display 1
		PORTF = seg7c[u];
		_delay_ms(3);
		
		PORTC = 0x02;			//Display 2
		PORTF = seg7c[d];
		_delay_ms(3);
		
		PORTC = 0x04;			//Display 3
		PORTF = seg7c[c];
		_delay_ms(3);
	}
	
	
}
	


	


