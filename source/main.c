/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *	CHECK VIDEO DEMO REQUIREMENTS 
 */ 
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


/*
 *
 * PC0 1 2 3 A
 * PC1 4 5 6 B
 * PC2 7 8 9 C
 * PC3 * 0 # D
*/


unsigned char GetKeypadKey(){
	PORTC = 0xEF;
	asm("nop");
	if ((PINC & 1)==0) { return('1');}
	if ((PINC & 2)==0) { return('4');}
	if ((PINC & 4)==0) { return('7');}
	if ((PINC & 8)==0) { return('*');}

	PORTC = 0xDF;
	asm("nop");
	if ((PINC & 1)==0) { return('2');}
	if ((PINC & 2)==0) { return('5');}
	if ((PINC & 4)==0) { return('8');}
	if ((PINC & 8)==0) { return('0');}

	PORTC = 0xBF;
	asm("nop");
	if ((PINC & 1)==0) { return('3');}
	if ((PINC & 2)==0) { return('6');}
	if ((PINC & 4)==0) { return('9');}
	if ((PINC & 8)==0) { return('#');}

	PORTC = 0x7F;
	asm("nop");
	if ((PINC & 1)==0) { return('A');}
	if ((PINC & 2)==0) { return('B');}
	if ((PINC & 4)==0) { return('C');}
	if ((PINC & 8)==0) { return('D');}

	return('\0');
}

int main(void)
{
	unsigned char x;
	DDRD = 0xFF; PORTD = 0x00; 
	DDRC = 0xF0; PORTC = 0x0F;
	while(1){
		x = GetKeypadKey();
		switch(x) {
			case '\0': PORTD = 0x1F; break;
			case '1': PORTD = 0x01; break;
			case '2': PORTD = 0x02; break;
			case '3': PORTD = 0x03; break;
			case '4': PORTD = 0x04; break;
			case '5': PORTD = 0x05; break;
			case '6': PORTD = 0x06; break;
			case '7': PORTD = 0x07; break;
			case '8': PORTD = 0x08; break;
			case '9': PORTD = 0x09; break;
			case 'A': PORTD = 0x0A; break;
			case 'B': PORTD = 0x0B; break;
			case 'C': PORTD = 0x0C; break;
			case 'D': PORTD = 0x0D; break;
			case '*': PORTD = 0x0E; break;
			case '0': PORTD = 0x00; break;
			case '#': PORTD = 0x0F; break;
			default: PORTD = 0x1B; break;
		}
	}
}
