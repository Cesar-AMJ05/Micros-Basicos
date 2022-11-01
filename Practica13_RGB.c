/*
 * P13_RGB.c
 *
 * Created: 30/10/2022 04:11:25 p. m.
 * Author : cesar y toño
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd/lcd.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include "lcd/lcd.h" //Puerto C
//Valores OCRnA/B/C
uint16_t r=0,g=0,b=0;
//PWM %
uint8_t rp=0, gp=0, bp=0, dumi;
//Lcd
char arraybot[16];
char arraytop[16];

void Int_FPWM_EXT(void){
	//MODO FAST MODE PWM 16Bits 
	TCCR4A|=(1<<WGM41)|(1<<COM4A1)|(1<<COM4B1)|(1<<COM4C1);//Salidas OCR4/A/B/C
	TCCR4B|=(1<<CS41)|(1<<CS40)|(1<<WGM42)|(1<<WGM43);//Prescaler 64 y modo Fast PWM modo 14 (Icrn TOP)
	TCCR4C=0x00;
	ICR4=49;// Se obtiene de la ecuación f_pwm=fosc/N(1+TOP), para la frecuencia de 6kHz y el prescaler de  64 nos un TOP=49
	TCNT4H=0x00; TCNT4L=0x00;//Se cuenta desde 0x0000
	OCR4A=0; OCR4B=0; OCR4C=0;//Se inician las salidas en 0
	//BOTONES
	EICRA|=(1<<ISC00)|(1<<ISC01)|(1<<ISC10)|(1<<ISC11)|(1<<ISC20)|(1<<ISC21);//Interrupciones externas 0/1/2 en pull up
	EIMSK|=(1<<INT0)|(1<<INT1)|(1<<INT2);
	sei();	
}

void Puertos(void){
	DDRH|=(1<<DDH3)|(1<<DDH4)|(1<<DDH5);
	PORTH&=(~(1<<PH3))|(~(1<<PH4))|(~(1<<PH5));//Se declaran las salidas del PWM 
}

void set(void){
	OCR4A=r; OCR4B=g; OCR4C=b; //Se configura el PWM, ojo el Atmel (Microchip Studio) divide el nible alto y bajo por la misma libreria
	TCNT4H=0x00; TCNT4L=0x00;
}

void lcd_output(void){//Salida datos a LCD
	lcd_clrscr();
	sprintf(arraytop,"RGB PWM R=%d%%",rp);
	sprintf(arraybot,"G=%d%%,B=%d%%",gp,bp);
	lcd_puts(arraytop);
	lcd_gotoxy(0,1);
	lcd_puts(arraybot);
}

//RED
ISR(INT0_vect){
	if((r<48)){//Cuenta maxima
		rp=rp+5;//Aumenta el porcetnaje
		if(rp%2==0){r=(r+2)+1;}//Al ser un numero flotante, se compenza sumando 1 cada 2 pulsos, asi se llega a los valores establecidos
		else{r=r+2;}
	}
	else{
		r=0;
		rp=0;
		set();
	}
	set();
	lcd_output();
}
//GREEN
ISR(INT2_vect){
	if((g<48)){
		gp+=5;
		if(gp%2==0){g=(g+2)+1;}
		else{g=g+2;}
	}
	else{
		g=0;
		gp=0;
	}
	set();
	lcd_output();
}
//BLUE
ISR(INT1_vect){
	if((b<48)){
		bp+=5;
		if(bp%2==0){b=(b+2)+1;}
		else{b=b+2;}
	}
	else{
		b=0; 
		bp=0;
	}
	set();
	lcd_output();
}

int main(void)
{	Int_FPWM_EXT();
	Puertos();
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_home();
	lcd_output();

    while (1) 
    {
	
    }
}

