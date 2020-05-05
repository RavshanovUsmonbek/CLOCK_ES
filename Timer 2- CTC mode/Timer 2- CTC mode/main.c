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
typedef unsigned char uch;
typedef unsigned int ui;

//////////////// Normal Clock's vars //////////////////////////////
uch sec, cnt, min, hour, month, day, week_day;
ui year;
uch mode = 0; 
uch step_time_set=1;
//////////////////////////////////////////////////////////////////////////

/////////////////// Stopwatch's vars ///////////////////////////////////
ui cnt_stp_w, sec_stp_w, min_stp_w, hour_stp_w, is_stopped=1;
//////////////////////////////////////////////////////////////////////////

///////////////////////// ALARM CLOCK VARS //////////////////////////////////
char* month_arr[12]={"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
char* week_arr[7]={"MON","TUE", "WEN", "THU", "FRI", "SAT", "SUN"};
// clock data stored in these vars
char min_alarm=0, hour_alarm=0, day_alarm=0, month_alarm=0;
ui year_alarm=2020;
uch step = 1;
uch isset_alarm = 0;
uch temp_arr[4]={0,0,1,1};
ui temp_year=2020;
uch is_current_date=0;
/////////////////////////////////////////////////////////////////

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
int month_day_count(ui month, ui year)
{
	ui const month_days[]={31,28,31,30,31,30,31,31,30,31,30,31};
	if (month==2 && is_leap_year(year))
	{
		return 29;
	}
	return month_days[month-1];
}
void disable_alarm_clock(void)
{
			min_alarm=0;
			hour_alarm=0;
			day_alarm=0;
			month_alarm=0;
			year_alarm=0;
			step=1;
			isset_alarm=0;
			temp_year=year;
			
			// clearing temp variables
			for (uch i=0;i<4;i++)
			{
				temp_arr[i]=0;
				if (i>1)
				{
					temp_arr[i]=1;
				}
			}
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
		if(day>month_day_count(month,year))
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
uch alarm_clock_check_logic(uch min, uch hour, uch day, uch month, ui year)
{
	if(min_alarm==min && hour_alarm==hour && day_alarm==day && month_alarm==month && year_alarm==year)
	{
		return 1;
	}
	return 0;
}
void led_blink(void)  // led on and off all bits for INT1
{
	PORTB = 0xff;
	_delay_ms(300);
	PORTB = 0x00;
	_delay_ms(300);
}
void alarm_clock_display(void)
{
	if (!isset_alarm)
	{
		switch(step)
		{
			case 1:
			LCD_pos(2,0);
			LCD_STR("ALARM | YEAR ");
			LCD_pos(6,1);
			LCD_CHAR((temp_year)/1000+'0');
			LCD_CHAR((temp_year/100)%10+'0');
			LCD_CHAR((temp_year/10)%10+'0');
			LCD_CHAR((temp_year)%10+'0');
			break;
			
			case 2:
			LCD_pos(2,0);
			LCD_STR("ALARM | MONTH ");
			LCD_pos(6,1);
			LCD_STR(month_arr[temp_arr[3]-1]);
			break;
			
			case 3:
			LCD_pos(2,0);
			LCD_STR("ALARM | DAY ");
			LCD_pos(7,1);
			LCD_CHAR((temp_arr[2]/10)+'0');
			LCD_CHAR((temp_arr[2]%10)+'0');
			break;
			
			case 4:
			LCD_pos(2,0);
			LCD_STR("ALARM | HOUR");
			LCD_pos(7,1);
			LCD_CHAR((temp_arr[1]/10)+'0');
			LCD_CHAR((temp_arr[1]%10)+'0');
			break;

			case 5:
			LCD_pos(1,0);
			LCD_STR("ALARM | MINUTE");
			LCD_pos(6,1);
			LCD_CHAR((temp_arr[0]/10)+'0');
			LCD_CHAR((temp_arr[0]%10)+'0');

			case 7:
			break;
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
	if(mode==0)
	{
		mode = 1; // setting up stop_watch mode
		cnt_stp_w=0;
		sec_stp_w=0;
		min_stp_w=0;
		hour_stp_w=0;
	}
	else if(mode==1) // stopwatch
	{
		mode = 0;
		is_stopped=1;
	}
	else if(mode==2) // alarm clock
	{
		switch(step)
		{
			case 1:
			temp_year++;
			break;
			
			case 2:
			if (is_current_date==1)
			{
				if (temp_arr[3]+1>month && (temp_arr[3]+1)<=12)
				{
					temp_arr[3]++;
				}
			}
			else
			{
				if (temp_arr[3]+1>12)
				{
					temp_arr[3]=1;
				}
				else
				{
					temp_arr[3]++;
				}
			}
			break;
			
			case 3:
			if (is_current_date==1)
			{
				if (temp_arr[2]+1>day && temp_arr[2]+1<=month_day_count(temp_arr[3], temp_year))
				{
					temp_arr[2]++;
				}
			}
			else
			{
				if (temp_arr[2]+1>month_day_count(temp_arr[3], temp_year))
				{
					temp_arr[2]=1;
				}
				else
				{
					temp_arr[2]++;
				}
			}
			break;
			
			case 4: 
			if (is_current_date==1)
			{
				if (temp_arr[1]+1>hour && (temp_arr[1]+1)<=23)
				{
					temp_arr[1]++;
				}
			}
			else
			{
				if (temp_arr[1]+1>=24)
				{
					temp_arr[1]=0;
				}
				else
				{
					temp_arr[1]++;
				}
			}
			break;
			
			case 5:	
			if (is_current_date==1)
			{
				if (temp_arr[0]+1>min && (temp_arr[0]+1)<=59)
				{
					temp_arr[0]++;
				}
			} 
			else
			{
				if (temp_arr[0]+1>=60)
				{
					temp_arr[0]=0;
				}
				else
				{
					temp_arr[0]++;
				}	
			}
			break;
			default: break;
		}
		
	}
	else if (mode==3) // time setting mode
	{
		switch(step_time_set)
		{
			case 1:
			year++;
			break;
			
			case 2:
			if (month+1>12)
			{
				month=1;
			}
			else
			{
				month++;
			}
			break;
			
			case 3:

			if (day+1>month_day_count(month, year))
			{
				day=1;
			}
			else
			{
				day++;
			}

			break;
			case 4:
			if (hour+1>=24)
			{
				hour=0;
			}
			else
			{
				hour++;
			}
			break;
			
			case 5:
			if (min+1>=60)
			{
				min=0;
			}
			else
			{
				min++;
			}
			break;

			case 6:
			if (week_day+1>=7)
			{
				week_day=1;
			} 
			else
			{
				week_day++;
			}
			break;
			default: break;
		}
		
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
		mode=2;
	}
	else if(mode==2)
	{
		mode=0;
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
		disable_alarm_clock();
	}
	else if(mode==2)
	{
		switch(step)
		{
			case 1:
			if (temp_year-1>=year)
			{
				temp_year--;
			}
			break;
			
			case 2:
			if (is_current_date==1)
			{
				if (temp_arr[3]-1>=month)
				{
					temp_arr[3]--;
				}
			}
			else
			{
				if (temp_arr[3]-1<=0)
				{
					temp_arr[3]=12;
				}
				else
				{
					temp_arr[3]--;
				}
			}
			break;
			
			case 3:
			if (is_current_date==1)
			{
				if (!(temp_arr[2]-1<day))
				{
					temp_arr[2]--;
				}
			}
			else
			{
				if (temp_arr[2]-1<=0)
				{
					temp_arr[2]=month_day_count(temp_arr[3],temp_year);
				}
				else
				{
					temp_arr[2]--;
				}
			}
			break;

			case 4:
			if (is_current_date==1)
			{

				if (!(temp_arr[1]-1<hour))
				{
					temp_arr[1]--;
				}
			}
			else
			{
				if (temp_arr[1]-1<0)
				{
					temp_arr[1]=23;
				}
				else
				{
					temp_arr[1]--;
				}
			}
			break; 

			case 5:	
			if (is_current_date==1)
			{

				if (!(temp_arr[0]-1<min))
				{
					temp_arr[0]--;
				}
			}
			else
			{				
				if (temp_arr[0]-1<0)
				{
					temp_arr[0]=59;
				}
				else
				{
					temp_arr[0]--;
				}
			}
			break;

			default: break;
		}
	}
	else if (mode==3) // time setting mode for decreasing values
	{
		switch(step_time_set)
		{
			case 1:
				year--;
			break;
			
			case 2:
			if (month-1<=0)
			{
				month=12;
			}
			else
			{
				month--;
			}
			break;
			
			case 3:
			if (day-1<=0)
			{
				day=month_day_count(month,year);
			}
			else
			{
				day--;
			}
			break;

			case 4:
			if (hour-1<0)
			{
				hour=23;
			}
			else
			{
				hour--;
			}
			break;

			case 5:
			if (min-1<0)
			{
				min=59;
			}
			else
			{
				min--;
			}
			break;

			case 6:
			if (week_day-1<1)
			{
				week_day=1;
			} 
			else
			{
				week_day--;
			}
			break;
			default: LCD_Clear();
					LCD_STR("STEP has incorrect value");
		 break;
		}
	}
}

ISR(INT3_vect)
{
	if (mode==0)
	{
		mode=3; // time setting mode
	}
	else if(mode==1)
	{
		// code for stopwatch
	}
	else if(mode==2)
	{
		step++;
		switch(step)
		{
			case 2:
			if (temp_year==year)
			{
				temp_arr[3]=month;
				is_current_date=1;
			}
			else
			{
				is_current_date=0;
			}

			break;

			case 3:
			if (temp_arr[3]==month && is_current_date==1)
			{
				is_current_date=1;
				temp_arr[2]=day;
			}
			else
			{
				is_current_date=0;
			}
	
			break;
			
			case 4:
			if (temp_arr[2]==day && is_current_date==1)
			{
				is_current_date=1;
				temp_arr[1]=hour;
			}
			else{is_current_date=0;}
			break;

			case 5:
			if (temp_arr[1]==hour && is_current_date==1)
			{
				is_current_date=1;
				temp_arr[0]=min;
			}
			else{is_current_date=0;}
			break;
		}
		if (step>5)
		{
			step=1; // this means end of time setting
			isset_alarm=1;
				
			min_alarm=temp_arr[0];
			hour_alarm=temp_arr[1];
			day_alarm=temp_arr[2];
			month_alarm=temp_arr[3];
			year_alarm=temp_year;
				
			// clearing temp variables
			for (uch i=0;i<4;i++)
			{
				temp_arr[i]=0;
				if (i>1)
				{
					temp_arr[i]=1;
				}
			}
		}
	}
	else if (mode==3)
	{
		step_time_set++;
	
		if (step_time_set>6)
		{
			step_time_set=1; // this means end of time setting
			mode=0;
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

void display_alarm_time(void)
{
	if (isset_alarm)
	{
		LCD_pos(2,0);
		LCD_STR("ALARM IS ON");
		
		LCD_pos(0,1);
		LCD_CHAR((year_alarm)/1000+'0');
		LCD_CHAR((year_alarm/100)%10+'0');
		LCD_CHAR((year_alarm/10)%10+'0');
		LCD_CHAR((year_alarm)%10+'0');
		LCD_CHAR(' ');
		if ((day_alarm/10)>0)
		{
			LCD_CHAR((day_alarm/10)+'0');
			LCD_CHAR((day_alarm%10)+'0');
		}
		else
		{
			LCD_CHAR('0');
			LCD_CHAR((day_alarm)+'0');
		}
		LCD_CHAR('/');
		
		if ((month_alarm/10)>0)
		{
			LCD_CHAR((month_alarm/10)+'0');
			LCD_CHAR((month_alarm%10)+'0');
		}
		else
		{
			LCD_CHAR('0');
			LCD_CHAR((month_alarm)+'0');
		}
		LCD_CHAR(' ');
		LCD_CHAR((hour_alarm)/10+'0');
		LCD_CHAR((hour_alarm)%10+'0');
		LCD_CHAR(':');
		LCD_CHAR((min_alarm)/10+'0');
		LCD_CHAR((min_alarm)%10+'0');
		
	}
	else
	{
		alarm_clock_display();
	}
}
void time_setup(void)
{
	switch(step_time_set)
		{
		case 1:
		LCD_pos(2,0);
		LCD_STR("TIME | YEAR ");
		LCD_pos(6,1);
		LCD_CHAR((year)/1000+'0');
		LCD_CHAR((year/100)%10+'0');
		LCD_CHAR((year/10)%10+'0');
		LCD_CHAR((year)%10+'0');
		break;
		
		case 2:
		LCD_pos(2,0);
		LCD_STR("TIME | MONTH ");
		LCD_pos(6,1);
		LCD_STR(month_arr[month-1]);
		break;
		
		case 3:
		LCD_pos(2,0);
		LCD_STR("TIME | DAY ");
		LCD_pos(7,1);
		LCD_CHAR((day/10)+'0');
		LCD_CHAR((day%10)+'0');
		break;
		
		case 4:
		LCD_pos(2,0);
		LCD_STR("TIME | HOUR");
		LCD_pos(7,1);
		LCD_CHAR((hour/10)+'0');
		LCD_CHAR((hour%10)+'0');
		break;

		case 5:
		LCD_pos(1,0);
		LCD_STR("TIME | MINUTE");
		LCD_pos(6,1);
		LCD_CHAR((min/10)+'0');
		LCD_CHAR((min%10)+'0');
		break;
		case 6:
		LCD_pos(1,0);
		LCD_STR("TIME | WEEKDAY");
		LCD_pos(6,1);
		LCD_STR(week_arr[week_day-1]);
		case 7:
		break;
	}


}

int main(void)
{
	cnt=0;
	sec=0;
	min=10;
	hour=12;
	day = 15;
	week_day=5;
	month = 5;
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
	char prev_step = step;
	char prev_step_time= step_time_set;
	char was_alarm_on =0;
    /* Replace with your application code */
    while (1) 
    {
		if(mode!=prev)
		{
			LCD_Clear();
			prev=mode;
		}
		if (alarm_clock_check_logic(min,hour,day,month,year)==1)
		{
			led_blink();
			was_alarm_on=1;
		}
		if (was_alarm_on==1 && alarm_clock_check_logic(min,hour,day,month,year)==0)
		{
			disable_alarm_clock();
			PORTB = 0xff;
			_delay_ms(2);
			was_alarm_on=0;
		}
		switch(mode)
		{
			case 0: display_normal_mode();break;
			case 1: display_stop_watch(); break;
			case 2:	
			if (prev_step!=step)
			{
				LCD_Clear();
				cursor_home();
				prev_step=step;
			}
			display_alarm_time();
			break;

			case 3:
				if (prev_step_time!=step_time_set)
				{
					LCD_Clear();
					cursor_home();
					prev_step_time=step_time_set;
				}
				time_setup();
			break;
			default: display_normal_mode(); break;
		}
				
    }
}

