// ���� �� �������������� �����������


#define F_CPU 8000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<stdint.h>
#include<avr/interrupt.h>
#include "Seven_seg.h"

unsigned char min = 0;
unsigned char hours = 0;


// ������� ������������� ���� TWI
void I2CInit(void)
{
	// ��������� TWI ������
	TWBR = 2;
	TWSR = (1 << TWPS1)|(1 << TWPS0); // ������������ �� 64
	TWCR |= (1 << TWEN); // ��������� ������ TWI
}

void I2CStart(void)
{
	// �������� ������� �����
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTA);
	// �������� ��������� ����� TWINT
	while(!(TWCR & (1 << TWINT)));
}

void I2CStop(void)
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTO); // �������� ������� ����
	while(TWCR & (1 << TWSTO)); // �������� ���������� �������� ������� ����
}

// ������� ������ ������ �� ����
uint8_t I2CWriteByte(uint8_t data)
{
	TWDR = data; // �������� ������ � TWDR
	TWCR = (1 << TWEN)|(1 << TWINT); // ����� ����� TWINT ��� ������ �������� ������
	while(!(TWCR & (1 << TWINT))); // �������� ���������� ��������
	// �������� �������
	if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28 || (TWSR & 0xF8) == 0x40)
	{
		// ���� ����� DS1307, ���� R/W � ������ ��������
		// � �������� �������������
		return 1;
	}
	else
	return 0; // ������
}
// ������� ������ ������ �� ����
uint8_t I2CReadByte(uint8_t *data,uint8_t ack)
{
	if(ack) // ������������� �������������
	{
		// ���������� ������������� ����� ������
		TWCR |= (1 << TWEA);
	}
	else
	{
		// ���������� ��������������� ����� ������
		// ������� ���������� �� �������� ������ ������
		// ������ ������������ ��� ������������� ���������� �����
		TWCR &= ~(1 << TWEA);
	}
	// ���������� ������ ������ ����� ������ TWINT
	TWCR |= (1 << TWINT);
	while(!(TWCR & (1 << TWINT))); // �������� ��������� ����� TWINT
	// �������� �������
	if((TWSR & 0xF8) == 0x58 || (TWSR & 0xF8) == 0x50)
	{
		// ����� ������ � ����������� �������������
		//  ���
		// ����� ������ � ����������� ���������������
		*data = TWDR; // ������ ������
		return 1;
	}
	else
	return 0; // ������
}

// ������� ������ ������ �� DS1307
uint8_t DS1307Read(uint8_t address,uint8_t *data)
{
	uint8_t res; // ���������
	I2CStart(); // �����
	res = I2CWriteByte(0b11010000); // ����� DS1307 + ��� W
	if(!res)    return 0; // ������
	// �������� ������ ������������ ��������
	res = I2CWriteByte(address);
	if(!res)    return 0; // ������
	I2CStart(); // ��������� �����
	res = I2CWriteByte(0b11010001); // ����� DS1307 + ��� R
	if(!res)    return 0; // ������
	// ������ ������ � ����������������
	res = I2CReadByte(data,0);
	if(!res)    return 0; // ������
	I2CStop(); // ����
	return 1;
}

// ������� ������ ������ � DS1307
uint8_t DS1307Write(uint8_t address,uint8_t data)
{
	uint8_t res; // ���������
	I2CStart(); // �����
	res = I2CWriteByte(0b11010000); // ����� DS1307 + ��� W
	if(!res)    return 0; // ������
	// �������� ������ ������������ ��������
	res = I2CWriteByte(address);
	if(!res)    return 0; // ������
	res = I2CWriteByte(data); // ������ ������
	if(!res)    return 0; // ������
	I2CStop(); // ����
	return 1;
}





int main(void)
{

//����������� ������ �0
TCCR0=~(1<<CS02);  		//����������� ��������
TCCR0|=(1<<CS01)|(1<<CS00);
TIMSK|=1<<TOIE0;			
TIFR|=1<<TOV0;

//����������� ������ �2
TCCR2|=(1<<CS02)|(1<<CS01)|(1<<CS00);
TIMSK|=(1<<TOIE2);
TIFR|=1<<TOV2;

sei(); // ��������� ���������� ����������

// ������������� �����

// ��� ���������� ����������
DDRB=0xFF;
PORTB=0x00;

// ������ ��� ���������� ��������
DDRC |= (1<<PC5)|(1<<PC4)|(1<<PC3)|(1<<PC2)|(1<<PC1)|(1<<PC0);
PORTC &= ~((1<<PC5)|(1<<PC4)|(1<<PC3)|(1<<PC2)|(1<<PC1)|(1<<PC0));

// ����������� ������
DDRD&=~((1<<PD0)|(1<<PD1));
PORTD|=(1<<PD0)|(1<<PD1);

// ��������� ��� �����
uint8_t temp = 0;
DS1307Read(0x00,&temp);
temp &= ~(1 << 7); // �������� 7 ���
DS1307Write(0x00,temp);

temp = 24;
temp = ((temp/10)<<4) + temp%10;
DS1307Write(0x01,temp);
temp = 15;
temp = ((temp/10)<<4) + temp%10;
DS1307Write(0x02,temp);
temp = 25;
temp = ((temp/10)<<4) + temp%10;
DS1307Write(0x04,temp);
temp = 4;
temp = ((temp/10)<<4) + temp%10;
DS1307Write(0x05,temp);

while(1)
{
DS1307Read(0x01,&temp); // ������ �������� �����
min = (((temp & 0xF0) >> 4)*10)+(temp & 0x0F);

DS1307Read(0x02,&temp); // ������ �������� �����
hours = (((temp & 0xF0) >> 4)*10)+(temp & 0x0F);
		 
SetValue(hours, min); 

// ��������� ������� � ������� ������ 

	if((PIND&(1<<PD0))==0) 		
		{
		_delay_ms(50); 				// ��������� ������� ���������
		if((PIND&(1<<PD0))==0) 
			{
			// ����������� ������
			min = min + 1;
			min = ((min/10)<<4) + min%10;
			DS1307Write(0x01,min);
	
			while((PIND&(1<<PD0))==0)
			{}
			}
		}
	if((PIND&(1<<PD1))==0)	
		{
		_delay_ms(50);			// ��������� ������� ���������
		if((PIND&(1<<PD1))==0)
			{	
			// ����������� ����
			hours = hours + 1;
			hours = ((hours/10)<<4) + hours%10;
			DS1307Write(0x02,hours);
			while((PIND&(1<<PD1))==0)
			{}
			}
		}
	
}
}
