
/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned long _avr_timer_M = 1; //start count from here, down to 0. Dft 1ms
unsigned long _avr_timer_cntcurr = 0; //Current internal count of 1ms ticks

void TimerOn(){
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit 3 = 0: CTC mode (clear timer on compare)
	//AVR output compare register OCR1A
	OCR1A = 125; // Timer interrupt will be generated when TCNT1 == OCR1A
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; //bit1: OCIE1A -- enables compare match interrupt
	//Init avr counter
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr ms
	
	//Enable global interrupts 
	SREG |= 0x80; //0x80: 1000000

}

void TimerOff(){
	TCCR1B = 0x00; //bit3bit1bit0 = 000: timer off
}


ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
			TimerISR();
			_avr_timer_cntcurr = _avr_timer_M;
			}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char tmpDT1;
unsigned char tmpDT2;
unsigned char tmpBT1;
unsigned char keyPressed;



typedef struct task {
  int state; // Current state of the task
  unsigned long period; // Rate at which the task should tick
  unsigned long elapsedTime; // Time since task's previous tick
  int (*TickFct)(int); // Function to call for task's tick
} task;

task tasks[1];

const unsigned char tasksNum = 1;


const unsigned long tasksPeriodGCD = 1;
const unsigned long periodKP = 100;


enum KP_States { KP_SMStart, KP_s1 };
int TickFct_KP(int state);


void TimerISR() {
  unsigned char i;
  for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
     if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
        tasks[i].state = tasks[i].TickFct(tasks[i].state);
        tasks[i].elapsedTime = 0;
     }
     tasks[i].elapsedTime += tasksPeriodGCD;
  }
}


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

int main() {

  DDRC = 0xF0; PORTC = 0x0F;
  DDRB = 0x80; PORTB = 0x00;
  unsigned char i=0;
  tasks[i].state = KP_SMStart;
  tasks[i].period = periodKP;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_KP;
  
  TimerOn();

  
  
  while(1) {
     PORTB = tmpBT1;
  }
  return 0;
}

int TickFct_KP(int state) {
  switch(state) { // Transitions
     case KP_SMStart: // Initial transition
        state = KP_s1;
        break;
     case KP_s1:
        state = KP_s1;
        break;
     default:
        state = KP_SMStart;
   } // Transitions

  switch(state) { // State actions
     case KP_s1:
	keyPressed = GetKeypadKey();
	if(keyPressed != '\0') tmpBT1 = 0x80;
	else tmpBT1 = 0;
        break;
     default:
        break;
  } // State actions
  //PORTD = tmpDT1;
  return state;
}
