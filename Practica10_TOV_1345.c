/*
 * Practica10_TOV_1345.c
 *
 * Created: 25/10/2022 10:05:58 a. m.
 * Author : Cesar y Antonio 
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

void It_set(void){
	TCCR1A=0x00;//Timer 1
	TCCR3A=0x00;//Timer 3
	TCCR4A=0x00;//Timer 4
	TCCR5A=0x00;//Timer 5
	TCCR1B|=(1<<CS12)|(1<<CS10);//pre 1024 T1
	TCCR3B|=(1<<CS32)|(1<<CS30);//pre 1024 T1
	TCCR4B|=(1<<CS42)|(1<<CS40);//pre 1024 T1
	TCCR5B|=(1<<CS52)|(1<<CS50);//pre 1024 T1
	TIMSK1|=(1<<TOIE1);//Mascara TOIE1
	TIMSK3|=(1<<TOIE3);//Mascara TOIE3
	TIMSK4|=(1<<TOIE4);//Mascara TOIE4
	TIMSK5|=(1<<TOIE5);//Mascara TOIE5
	sei();
}
void Puertos(void){
	DDRH|=(1<<DDH0);	PORTH|=(1<<PH0); //Verde
	DDRL|=(1<<DDL2);	PORTL|=(1<<PL2); //Rojo 
	DDRC|=(1<<DDC6);	PORTC|=(1<<PC6); //Azul
	DDRF|=(1<<DDF5);	PORTF|=(1<<PF5); //Blanco
}
void Sets_Initial(void){
	TCNT1H=0xC2; TCNT1L=0xF7; //1 sec Verde T0V1
	TCNT3H=0x85; TCNT3L=0xEE; //2 sec Rojo TOV3
	TCNT4H=0x48; TCNT4L=0xE5; //3 sec Azul TOV4
	TCNT5H=0x0B; TCNT5L=0xDC; //4 sec Blanco TOV5
}

ISR(TIMER1_OVF_vect){//Led  Verde
	PORTH^=(1<<PH0);
	TCNT1H=0xC2; TCNT1L=0xF7;
}

ISR(TIMER3_OVF_vect){//Led Rojo
	PORTL^=(1<<PL2);
	TCNT3H=0x85; TCNT3L=0xEE;
}

ISR(TIMER4_OVF_vect){//Led Azul
	PORTC^=(1<<PC6);
	TCNT4H=0x48; TCNT4L=0xE5; //3 sec Azul TOV4
}

ISR(TIMER5_OVF_vect){//Led Blanco
	PORTF^=(1<<PF5);
	TCNT5H=0x0B; TCNT5L=0xDC; 
}
int main(void){	
	It_set();
    Puertos();
	Sets_Initial();
    while (1) 
    {
    }
}

