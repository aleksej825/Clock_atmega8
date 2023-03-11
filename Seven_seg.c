/*
 * _7_seg.c
 *
 * Created: 4/3/2022 1:35:28 PM
 *  Author: Aleksey
 */ 

#include "Seven_seg.h"
//#include <avr/io.h>

// Array of numbers
unsigned char number[]=
{
	0x3F,//0
	0x06,//1
	0x5B,//2
	0x4F,//3
	0x66,//4
	0x6D,//5
	0x7D,//6
	0x07,//7
	0x7F,//8
	0x6F //9
};

volatile unsigned char data1=0;
volatile unsigned char data2=0;
volatile unsigned char data3=0;
volatile unsigned char data4=0;

static uint8_t point = 0x80;

// Dynamic indication timer
ISR(TIMER0_OVF_vect)
{
	static unsigned char clock = 0;

	PORTC |=((1<<PC3)|(1<<PC2)|(1<<PC1)|(1<<PC0));
	if(clock==0)
	{
		PORTB=number[data4];
		PORTC &=~(1<<PC3);
	}
	if(clock==1)
	{
		PORTB=number[data3];
		PORTC &=~(1<<PC2);
	}
	if(clock==2)
	{
		PORTB=number[data2]|point;
		PORTC&=~(1<<PC1);
	}
	if(clock==3)
	{
		PORTB=number[data1];
		PORTC &=~(1<<PC0);
	}
	clock++;
	if(clock==4)
	clock=0;
	TIFR|=1<<TOV0;
}
// Point timer

ISR(TIMER2_OVF_vect)
{
	static uint8_t clock = 0;

	if(clock==33)
	{
		point = 0x00;	
	}
	if(clock==66)
	{
		point = 0x80;
		clock=0;
	}
	TIFR|=1<<TOV2;
}

void SetValue(uint8_t first, uint8_t second)
{
	if(first > 99)
		first = 99;

	if(second > 99)
		second = 99;

	data1 = first/10;
	data2 = first%10;
	data3 = second/10;
	data4 = second%10;
}