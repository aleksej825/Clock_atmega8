// Часы на семисегментных индикаторах


#define F_CPU 8000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<stdint.h>
#include<avr/interrupt.h>
unsigned char min = 0;
unsigned char hours = 0;
static uint8_t point = 0x80;

// Функция инициализация шины TWI
void I2CInit(void)
{
	// настройка TWI модуля
	TWBR = 2;
	TWSR = (1 << TWPS1)|(1 << TWPS0); // Предделитель на 64
	TWCR |= (1 << TWEN); // Включение модуля TWI
}

void I2CStart(void)
{
	// Передача условия СТАРТ
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTA);
	// Ожидание установки флага TWINT
	while(!(TWCR & (1 << TWINT)));
}

void I2CStop(void)
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTO); // Передача условия СТОП
	while(TWCR & (1 << TWSTO)); // Ожидание завершения передачи условия СТОП
}

// Функция записи данных по шине
uint8_t I2CWriteByte(uint8_t data)
{
	TWDR = data; // Загрузка данных в TWDR
	TWCR = (1 << TWEN)|(1 << TWINT); // Сброс флага TWINT для начала передачи данных
	while(!(TWCR & (1 << TWINT))); // Ожидание завершения передачи
	// Проверка статуса
	if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28 || (TWSR & 0xF8) == 0x40)
	{
		// Если адрес DS1307, биты R/W и данные переданы
		// и получено подтверждение
		return 1;
	}
	else
	return 0; // ОШИБКА
}
// Функция чтения данных по шине
uint8_t I2CReadByte(uint8_t *data,uint8_t ack)
{
	if(ack) // Устанавливаем подтверждение
	{
		// Возвращаем подтверждение после приема
		TWCR |= (1 << TWEA);
	}
	else
	{
		// Возвращаем неподтверждение после приема
		// Ведомое устройство не получает больше данных
		// обычно используется для распознования последнего байта
		TWCR &= ~(1 << TWEA);
	}
	// Разрешение приема данных после сброса TWINT
	TWCR |= (1 << TWINT);
	while(!(TWCR & (1 << TWINT))); // Ожидание установки флага TWINT
	// Проверка статуса
	if((TWSR & 0xF8) == 0x58 || (TWSR & 0xF8) == 0x50)
	{
		// Прием данных и возвращение подтверждения
		//  или
		// Прием данных и возвращение неподтверждения
		*data = TWDR; // Читаем данные
		return 1;
	}
	else
	return 0; // Ошибка
}

// Функция чтения данных из DS1307
uint8_t DS1307Read(uint8_t address,uint8_t *data)
{
	uint8_t res; // Результат
	I2CStart(); // СТАРТ
	res = I2CWriteByte(0b11010000); // адрес DS1307 + бит W
	if(!res)    return 0; // ОШИБКА
	// Передача адреса необходимого регистра
	res = I2CWriteByte(address);
	if(!res)    return 0; // ОШИБКА
	I2CStart(); // Повторный СТАРТ
	res = I2CWriteByte(0b11010001); // адрес DS1307 + бит R
	if(!res)    return 0; // ОШИБКА
	// Чтение данных с неподтверждением
	res = I2CReadByte(data,0);
	if(!res)    return 0; // ОШИБКА
	I2CStop(); // СТОП
	return 1;
}

// Функция записи данных в DS1307
uint8_t DS1307Write(uint8_t address,uint8_t data)
{
	uint8_t res; // Результат
	I2CStart(); // СТАРТ
	res = I2CWriteByte(0b11010000); // адрес DS1307 + бит W
	if(!res)    return 0; // ОШИБКА
	// Передача адреса необходимого регистра
	res = I2CWriteByte(address);
	if(!res)    return 0; // ОШИБКА
	res = I2CWriteByte(data); // Запись данных
	if(!res)    return 0; // ОШИБКА
	I2CStop(); // СТОП
	return 1;
}
// Масив чисел
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

// Таймер для динамической индикации
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
// Таймер для точки

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

int main(void)
{

//Настраиваем таймер Т0
TCCR0=~(1<<CS02);  		//Настраиваем делитель
TCCR0|=(1<<CS01)|(1<<CS00);
TIMSK|=1<<TOIE0;			
TIFR|=1<<TOV0;

//Настраиваем таймер Т2
TCCR2|=(1<<CS02)|(1<<CS01)|(1<<CS00);
TIMSK|=(1<<TOIE2);
TIFR|=1<<TOV2;

sei(); // Разришаем глобальные прерывания

// Конфигурируем порты

// Для управления сегментами
DDRB=0xFF;
PORTB=0x00;

// Выводы для управления катодами
DDRC |= (1<<PC5)|(1<<PC4)|(1<<PC3)|(1<<PC2)|(1<<PC1)|(1<<PC0);
PORTC &= ~((1<<PC5)|(1<<PC4)|(1<<PC3)|(1<<PC2)|(1<<PC1)|(1<<PC0));

// Подключение кнопок
DDRD&=~((1<<PD0)|(1<<PD1));
PORTD|=(1<<PD0)|(1<<PD1);

// Запускаем ход часов
uint8_t temp = 0;
DS1307Read(0x00,&temp);
temp &= ~(1 << 7); // обнуляем 7 бит
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
DS1307Read(0x01,&temp); // Чтение регистра минут
min = (((temp & 0xF0) >> 4)*10)+(temp & 0x0F);

DS1307Read(0x02,&temp); // Чтение регистра часов
hours = (((temp & 0xF0) >> 4)*10)+(temp & 0x0F);
		  
data1=hours/10;
data2=hours%10;
data3=min/10;
data4=min%10;

// Изменение времени с помощью кнопки 

	if((PIND&(1<<PD0))==0) 		
		{
		_delay_ms(50); 				// Устраняем дребезг контактов
		if((PIND&(1<<PD0))==0) 
			{
			// Увеличиваем минуты
			min = min + 1;
			min = ((min/10)<<4) + min%10;
			DS1307Write(0x01,min);
	
			while((PIND&(1<<PD0))==0)
			{}
			}
		}
	if((PIND&(1<<PD1))==0)	
		{
		_delay_ms(50);			// Устраняем дребезг контактов
		if((PIND&(1<<PD1))==0)
			{	
			// Увеличиваем часы
			hours = hours + 1;
			hours = ((hours/10)<<4) + hours%10;
			DS1307Write(0x02,hours);
			while((PIND&(1<<PD1))==0)
			{}
			}
		}
	
}
}
