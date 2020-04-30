/*
 * lcd.h
 *
 * Created: 4/11/2020 7:54:18 PM
 *  Author: USMON
 */ 


#ifndef LCD_H_
#define LCD_H_
 
 #define LCD_WDATA PORTA // LCD ??? ?? ??
 #define LCD_WINST PORTA
 #define LCD_CTRL PORTG // LCD ???? ??
 #define LCD_EN 0
 #define LCD_RW 1
 #define LCD_RS 2
 #define RIGHT 1
 #define LEFT 0
#include "_main.h"

 void PortInit(void)
 {
	 DDRA = 0xFF; 
	 DDRG = 0x0F; 
 }
void LCD_Data(char ch) 	            
{
	LCD_CTRL |= (1 << LCD_RS);       
	LCD_CTRL &= ~(1 << LCD_RW);
	LCD_CTRL |= (1 << LCD_EN);       
	_delay_us(50);                               
	LCD_WDATA = ch; 
	_delay_us(50);                                
	LCD_CTRL &= ~(1 << LCD_EN);    
}
void LCD_Comm(char ch)	            
{
	LCD_CTRL &= ~(1 << LCD_RS);   
	LCD_CTRL &= ~(1 << LCD_RW);
	LCD_CTRL |= (1 << LCD_EN);      
	_delay_us(50);                               
	LCD_WINST = ch;                         
	_delay_us(50);                              
	LCD_CTRL &= ~(1 << LCD_EN);   
}

void LCD_CHAR(char c) 	
{
	LCD_Data(c);
	_delay_ms(2);
}

void LCD_STR(char *str) 	
{
	while(*str != 0) {
		LCD_CHAR(*str);
		str++;
	}
}

void LCD_pos(unsigned char row, unsigned char col)
{
	LCD_Comm(0x80|(row+col*0x40)); 
}

void LCD_Clear(void)	
{
	LCD_Comm(0x01);
	_delay_ms(2);
}
void LCD_Init(void) 	
{
	LCD_Comm(0x38);	
	_delay_ms(2); 		
	LCD_Comm(0x38); 	
	_delay_ms(2);		
	LCD_Comm(0x38); 	
	_delay_ms(2);		
	LCD_Comm(0x0e);	
	_delay_ms(2);		
	LCD_Comm(0x06);	
	_delay_ms(2);		
	LCD_Clear();	
}

void LCD_Shift(char p)
{
	if(p == RIGHT) {
		LCD_Comm(0x1C); 
		_delay_ms(1); 
	}
	else if(p == LEFT) { 
		LCD_Comm(0x18); 
		_delay_ms(1);

	}	
}

void cursor_home(void) {
	LCD_Comm(0x02); 
	_delay_ms(2); 
}
#endif /* LCD_H_ */