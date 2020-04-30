/*
 * Timer 2- CTC mode.c
 *
 * Created: 4/21/2020 5:57:08 PM
 * Author : USMON
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "_main.h"
#include "lcd.h"

char Temp;
int cnt;
char sec, min, hour, month, day, week_day;
unsigned int year;

void init_timer()
{
	TIMSK = 0x02; // output compare interrupt enabled
	TCCR0 = 0x0f; // CTC mode, prescale 1024
	OCR0 = 99;
}

char is_leap_year(int y) // utility function for determining leap year
{
	if(y % 4 == 0)
	{
		//Nested if else
		if( y % 100 == 0)
		{
			if ( y % 400 == 0)
				return 1;
			else
				return 0;
		}
		else
			return 1;
	}
	else
		return 0;
}
char month_day_count(int month)
{
	char short_month[]={4,6,9,11};
	
	if (month == 2)
	{
		if (is_leap_year(year)==1)
			return 29;
		else
			return 28;
	}
	
	for(char i=0;i<4;i++)
	{
		if(month==short_month[i])
			return 31;
	}
	return 30;	
}


ISR(TIMER0_COMP_vect)
{
	cnt++;
	if(cnt==144)
	{
		cnt=0;
		sec++;
		if (sec>=60)
		{
			min++;
			sec=0;
		}
		if (min==60)
		{
			hour++;
			min = 0;
		}
		if(hour>=24)
		{
			day++;
			week_day++;
			if (week_day==8)
				week_day=1;
			hour = 0;
		}
		if(day>=month_day_count(month))
		{
			month++;
			day = 1;
		}
		if(month>=12)
		{
			year++;
			month = 1;
		}			
	}
		
}

void display_normal_mode(void)
{
	char AM[] = "AM";
	char PM[] = "PM";
	// Displaying year
	LCD_pos(0,0);
	LCD_CHAR((year)/1000+'0');
	LCD_CHAR((year/100)%10+'0');
	LCD_CHAR((year/10)%10+'0');
	LCD_CHAR((year)%10+'0');
	
	
	// displaying day and month in form of "d/m"
	LCD_pos(5,0);
	if (day/10>0)
	{
		LCD_CHAR((day)/10+'0');
		LCD_CHAR((day)%10+'0');
	}
	else
	LCD_CHAR((day)+'0');

	LCD_CHAR('/');
	if (month/10>0)
	{
		LCD_CHAR((month)/10+'0');
		LCD_CHAR((month)%10+'0');
	}
	else
	LCD_CHAR((month)+'0');
	//////////////////////////////////////////////////////////////////////////
	
	// displaying week days
	LCD_pos(11,0);
	switch(week_day)
	{
		case 1: LCD_STR("Mon"); break;
		case 2: LCD_STR("Tue"); break;
		case 3: LCD_STR("Wen"); break;
		case 4: LCD_STR("Thu"); break;
		case 5: LCD_STR("Fri"); break;
		case 6: LCD_STR("Sat"); break;
		case 7: LCD_STR("Sun"); break;
	}
	//////////////////////////////////////////////////////////////////////////
	
	// displaying hours
	if(hour>12)
	{
		LCD_pos(0,1);
		LCD_STR(PM);
		LCD_pos(3,1);
		LCD_CHAR((hour-12)/10+'0');
		LCD_CHAR((hour-12)%10+'0');
		LCD_CHAR(':');
		
	}
	else
	{
		LCD_pos(0,1);
		LCD_STR(AM);
		LCD_pos(3,1);
		LCD_CHAR((hour)/10+'0');
		LCD_CHAR((hour)%10+'0');
		LCD_CHAR(':');
	}
	
	// min displaying
	LCD_pos(6,1);
	LCD_CHAR((min/10)+'0');
	LCD_CHAR((min%10)+'0');
	LCD_CHAR(':');
	
	// sec displaying
	LCD_pos(9, 1);
	LCD_CHAR((sec/10)+'0');
	LCD_CHAR((sec%10)+'0');
	
}

int main(void)
{
	Temp =0;
	cnt=0;
	sec=0;
	min=45;
	hour=18;
	day = 28;
	week_day=2;
	month = 4;
	year = 2020;
	init_timer();
	SREG |=0x80;
	
	// LCD init
	PortInit();
	LCD_Init();
	LCD_Clear();
	// cursor off display on
	LCD_Comm(0x0c);
	_delay_ms(2);
		
    /* Replace with your application code */
    while (1) 
    {
		display_normal_mode();

    }
}

