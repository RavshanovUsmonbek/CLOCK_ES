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


unsigned char mode = 0; // normal clock mode
//////////////// Normal Clock's vars //////////////////////////////
unsigned char sec, cnt, min, hour, month, day, week_day;
unsigned int year;
//////////////////////////////////////////////////////////////////////////

/////////////////// Stopwatch's vars ///////////////////////////////////
unsigned char cnt_stp_w, sec_stp_w, min_stp_w, hour_stp_w, is_stopped=1;
//////////////////////////////////////////////////////////////////////////

void init_timer()
{
	TIMSK = 0x02; // output compare interrupt enabled
	TCCR0 = 0x0f; // CTC mode, prescale 1024
	OCR0 = 143;
}
void port_init()
{
	DDRA = 0xff; // lcd port configuration of DDRA and DDRG
	DDRG = 0x0f;
	DDRB = 0xff; // making PORTB as an output port for leds
	PORTB= 0xff;
	DDRD = 0x00; // making PORTD as an input port for recieving signals form switches
}
void interrupt_init(void)
{
	EIMSK=0xff; // enabling all interrupts
	EICRA=0xaa;// enabling falling edge trigger for all interrupts
	EICRB=0xaa; 
	sei();   // setting global interrupt on
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
int month_day_count(unsigned int month)
{
	unsigned int const month_days[]={31,28,31,30,31,30,31,31,30,31,30,31};
	if (month==2 && is_leap_year(year))
	{
		return 29;
	}
	return month_days[month-1];
}
void normal_clock_logic(void)
{
	cnt++;
	if(cnt==100)
	{
		cnt=0;
		sec++;
		if (sec>=60)
		{
			min++;
			sec=0;
		}
		if (min>=60)
		{
			hour++;
			min = 0;
		}
		if(hour>=24)
		{
			day++;
			week_day++;
			hour = 0;
		}
		if (week_day>=8)
		{
			week_day=1;
		}
		if(day>month_day_count(month))
		{
			month++;
			day = 1;
		}
		if(month>12)
		{
			year++;
			month = 1;
		}
	}
}
void stop_watch_logic(void)
{
	cnt_stp_w++;
	if(cnt_stp_w>=100)
	{
		cnt_stp_w=0;
		sec_stp_w++;
		if (sec_stp_w>=60)
		{
			min_stp_w++;
			sec_stp_w=0;
		}
		if (min_stp_w>=60)
		{
			hour_stp_w++;
			min_stp_w = 0;
		}
		if(hour_stp_w>=24)
		{
			hour_stp_w = 0;
		}
	}
}
	
ISR(TIMER0_COMP_vect)
{
	if(!is_stopped)
		stop_watch_logic();
		
	normal_clock_logic();
}

ISR(INT0_vect)
{
	switch(mode)
	{
		case 0: 
				mode = 1; // setting up stop_watch mode
				cnt_stp_w=0;
				sec_stp_w=0;
				min_stp_w=0;
				hour_stp_w=0;
				break;
		
		case 1:	mode = 0;
			    is_stopped=1;
				break;
				
		case 2:	break;
		
		default: break;
	}
}

ISR(INT1_vect)
{
	if(mode==1) // stop and go button for stopwatch 
	{
		if (!is_stopped)
			is_stopped = 1;
		else
			is_stopped=0;
	}
	else if(mode==0)
	{
		// code for normal clock
	}
	else if(mode==2)
	{
		//code for alarm clock
	}
}

ISR(INT2_vect)
{
	if(mode==1)// reset button for stopwatch
	{
		if(!is_stopped)
			is_stopped=1;

		cnt_stp_w=0;
		sec_stp_w=0;
		min_stp_w=0;
		hour_stp_w=0;
	}
	else if(mode==0)
	{
		// code for normal clock
	}
	else if(mode==2)
	{
		//code for alarm clock
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
	if ((day/10)>0)
	{
		LCD_CHAR((day/10)+'0');
		LCD_CHAR((day%10)+'0');
	}
	else
	{
		LCD_CHAR('0');
		LCD_CHAR((day)+'0');
	}
	LCD_CHAR('/');
	
	if ((month/10)>0)
	{
		LCD_CHAR((month/10)+'0');
		LCD_CHAR((month%10)+'0');
	}
	else
	{
		LCD_CHAR('0');
		LCD_CHAR((month)+'0');
	}
	
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

void display_stop_watch(void)
{
	// hour
	LCD_pos(0,0);
	LCD_CHAR((hour_stp_w)/10+'0');
	LCD_CHAR((hour_stp_w)%10+'0');
	LCD_CHAR('h');
	
	// min
	LCD_pos(3,1);
	LCD_CHAR((min_stp_w/10)+'0');
	LCD_CHAR((min_stp_w%10)+'0');
	LCD_CHAR(':');
	
	// sec
	LCD_pos(6, 1);
	LCD_CHAR((sec_stp_w/10)+'0');
	LCD_CHAR((sec_stp_w%10)+'0');
	LCD_CHAR(':');
	
	//milliseconds
	LCD_pos(9, 1);
	LCD_CHAR((cnt_stp_w/10)+'0');
	LCD_CHAR((cnt_stp_w%10)+'0');
	
}

int main(void)
{
	
	cnt=0;
	sec=0;
	min=0;
	hour=23;
	day = 31;
	week_day=7;
	month = 12;
	year = 2020;
	
	init_timer();
	interrupt_init();
	
	// LCD init
	port_init();
	LCD_Init();
	LCD_Clear();
	LCD_Comm(0x0c);
	_delay_ms(2);
	char prev=mode;
    /* Replace with your application code */
    while (1) 
    {
		if(mode!=prev)
		{
			LCD_Clear();
			prev=mode;
		}
		switch(mode)
		{
			case 0: display_normal_mode();break;
			case 1: display_stop_watch(); break;
			case 2: break;
			default: display_normal_mode(); break;
		}
				
    }
}

