
/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: PORTB = tmpBT1;Lab #  Exercise #
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

void set_PWM(double frequency){
	static double current_frequency;

	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else {TCCR3B |= 0x03; }

		if (frequency < 0.954) { OCR3A = 0xFFFF; }

		else if (frequency > 31250) { OCR3A = 0x0000; }

		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0;
		current_frequency = frequency; 
	}
}

void PWM_on(){
	TCCR3A = (1 << COM3A0);

	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);

	set_PWM(0);
}

void PWM_off(){
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

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




typedef struct task {
  int state; // Current state of the task
  unsigned long period; // Rate at which the task should tick
  unsigned long elapsedTime; // Time since task's previous tick
  int (*TickFct)(int); // Function to call for task's tick
} task;

task tasks[4];

const unsigned char tasksNum = 4;


const unsigned long tasksPeriodGCD = 1;
const unsigned long periodKP = 100;
const unsigned long periodSQ = 50;
const unsigned long periodIS = 100;
const unsigned long periodSample = 200;

/*
const unsigned long periodThreeLEDs = 300;
unsigned long periodSpeaker = 2;
const unsigned long periodCombined = 1;
const unsigned long periodSample = 200;
*/

enum KP_States { KP_SMStart, KP_wait, KP_pressed };
int TickFct_KP(int state);

enum SQ_States { SQ_SMStart, SQ_init, SQ_begin, SQ_wait, SQ_check, SQ_match };
int TickFct_detectSQ(int state);


enum IS_States { IS_SMStart, IS_unlock, IS_lock };
int TickFct_IS(int state);


enum OnOff_States { OnOff_SMStart, s_Off, s_On};
int TickFct_OnOff(int state);
/*
enum SP_States { SP_SMStart, SP_s0, SP_s1};
int TickFct_Speaker(int state);

enum FRQ_States { FRQ_SMStart, FRQ_s0, FRQ_inc, FRQ_dec};
int TickFct_FRQ(int state);

*/

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

/*
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
*/

unsigned char GetKeypadKey(){
	unsigned char star = 0;
	PORTC = 0xEF;
	asm("nop");
	//if ((PINC & 8)==0) { return('*');}
	if ((PINC & 8)==0) { star = 1; }
	if ((PINC & 1)==0) {if(star == 1) return('a'); else return('1');}
	if ((PINC & 2)==0) {if(star == 1) return('d'); else return('4');}
	if ((PINC & 4)==0) {if(star == 1) return('g'); else return('7');}

	PORTC = 0xDF;
	asm("nop");
	if ((PINC & 1)==0) {if(star == 1) return('b'); else return('2');}
	if ((PINC & 2)==0) {if(star == 1) return('e'); else return('5');}
	if ((PINC & 4)==0) {if(star == 1) return('h'); else return('8');}
	if ((PINC & 8)==0) {if(star == 1) return('`'); else return('0');}

	PORTC = 0xBF;
	asm("nop");
	if ((PINC & 1)==0) {if(star == 1) return('c'); else return('3');}
	if ((PINC & 2)==0) {if(star == 1) return('f'); else return('6');}
	if ((PINC & 4)==0) {if(star == 1) return('i'); else return('9');}
	if ((PINC & 8)==0) { return('#');}

	PORTC = 0x7F;
	asm("nop");
	if ((PINC & 1)==0) { return('A');}
	if ((PINC & 2)==0) { return('B');}
	if ((PINC & 4)==0) { return('C');}
	if ((PINC & 8)==0) { return('D');}

	if(star == 1) return('*'); else return('\0');
}
//Queue Functions 
typedef struct _Q7uc {
   unsigned char buf[7];
   unsigned char cnt;
} Q7uc;

void Q7ucInit(Q7uc *Q) {
   (*Q).cnt=0;
}

unsigned char Q7ucFull(Q7uc Q) {
   return (Q.cnt == 7);
}

unsigned char Q7ucEmpty(Q7uc Q) {
   return (Q.cnt == 0);
}

void Q7ucPrint(Q7uc Q) {
   int j;
   printf("Q7uc contents: \r\n");
   for (j=0; j<7; j++) {
           printf("Item %d", j);
           printf(": %d\r\n", Q.buf[j]);
   }
}

void Q7ucPush(Q7uc *Q,
                  unsigned char item) {
   if (!Q7ucFull(*Q)) {
      //DisableInterrupts();
      (*Q).buf[(*Q).cnt] = item;
      (*Q).cnt++;
      //EnableInterrupts();
   }
}

unsigned char Q7ucPop(Q7uc *Q)
{
   int i;
   unsigned char item=0;
   if (!Q7ucEmpty(*Q)) {
      //DisableInterrupts();
      item = (*Q).buf[0];
      (*Q).cnt--;
      for (i=0; i<(*Q).cnt; i++) {
         // shift fwd
         (*Q).buf[i]=
         (*Q).buf[i+1];
      }
      //EnableInterrupts();
   }
   return(item);
}

unsigned char tmpBUL;
unsigned char t2unlock; //0 = locked    1 = unlocked
unsigned char tmpBT1;
unsigned char keyPressed;
Q7uc btnQ;


unsigned char sequ[] = {'#','1','2','3','4','5'};
//unsigned char sequ[6];
//sequ[0] = '#';
//sequ[1] = '1';
//sequ[2] = '2';
//sequ[3] = '3';
//sequ[4] = '4';
//sequ[5] = '5';
#define MAXCOUNT 30
double frequency[9] = {0, 261.63, 293.66, 329.63, 349.23, 392.0, 440.0, 493.88, 523.25};
unsigned short keySeq[MAXCOUNT] = {1,1,1,1,5,5,5,5,4,3,2,8,8,8,8,5,5,4,3,2,8,8,8,8,5,5,4,3,4,2};
unsigned short count = 0;


int main() {
 

  DDRA = 0x00; PORTA = 0x03;
  DDRC = 0xF0; PORTC = 0x0F;
  //DDRB = 0xC1; PORTB = 0x00;
  DDRB = 0x05; PORTB = 0x00;
  unsigned char i=0;
  tasks[i].state = OnOff_SMStart;
  tasks[i].period = periodSample;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_OnOff;
  ++i;
  tasks[i].state = KP_SMStart;
  tasks[i].period = periodKP;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_KP;
  ++i;
  tasks[i].state = SQ_SMStart;
  tasks[i].period = periodSQ;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_detectSQ;
  ++i;
  tasks[i].state = IS_SMStart;
  tasks[i].period = periodIS;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_IS;
  
  //TimerSet(tasksPeriodGCD);
  
  Q7ucInit(&btnQ);
  PWM_on();
  TimerOn();

  
  
  while(1) {
     PORTB = tmpBT1 | t2unlock;
  }
  return 0;
}

int TickFct_KP(int state) {
  switch(state) { // Transitions
     case KP_SMStart: // Initial transition
        state = KP_wait;
        break;
     case KP_wait:
	keyPressed = GetKeypadKey();
	if(keyPressed == '\0')
		state = KP_wait;
	else{ 
		state = KP_pressed;
		if(!Q7ucFull(btnQ)){
			Q7ucPush(&btnQ, keyPressed);
		}
	}
        break;
     case KP_pressed:
	keyPressed = GetKeypadKey();
	if(keyPressed == '\0')
		state = KP_wait;
	else 
		state = KP_pressed;
        break;
     default:
        state = KP_SMStart;
   } // Transitions
  switch(state) { // State actions
     case KP_wait:
        tmpBT1 = 0x00;
        break;
     case KP_pressed:
        tmpBT1 = 0x04;
	break;
     default:
        break;
  } // State actions
  //PORTD = tmpDT1;
  return state;
}

int TickFct_detectSQ(int state) {
  unsigned char recBtn;
  
//unsigned char sequ[6] = {'#','1','2','3','4','5'};
  static unsigned char i = 0;
  switch(state) { // Transitions
     case SQ_SMStart: // Initial transition
        state = SQ_init;
        break;
     case SQ_init:
        state = SQ_begin;
        break;
     case SQ_begin:
        state = SQ_wait;
        break;
     case SQ_wait:
	if(Q7ucEmpty(btnQ))state = SQ_wait;
	else{
		recBtn = Q7ucPop(&btnQ);
		state = SQ_check;
	}
        break;
     case SQ_check:
	if(recBtn == sequ[i])state = SQ_match;
	//if(recBtn == *(seq+i))state = SQ_match;
	else state = SQ_begin;
        break;
     case SQ_match:
	if(i!=5){
		i++;
		state = SQ_wait;
	}
	else{
		state = SQ_begin;
		t2unlock = 1;
	}
        break;
     default:
        state = SQ_SMStart;
   } // Transitions

  switch(state) { // state actions
     case SQ_SMStart: // 
        break;
     case SQ_init:
	t2unlock = 0;
        break;
     case SQ_begin:
	i = 0;
        break;
     case SQ_wait:
        break;
     case SQ_check:
	break;
     case SQ_match:
        break;
     default:
        state = SQ_SMStart;
   } // Transitions
  return state;
}


int TickFct_IS(int state) {
  switch(state) { // Transitions
     case IS_SMStart: // Initial transition
        state = IS_unlock;
        break;
     case IS_unlock:
	if(~PINA & 0x01 == 1) state = IS_lock;
	else state = IS_unlock;
        break;
     case IS_lock:
	if(~PINA & 0x01 == 1) state = IS_lock;
	else state = IS_unlock;
        break;
     default:
        state = IS_SMStart;
   } // Transitions
  switch(state) { // State actions
     case IS_unlock:
        break;
     case IS_lock:
	t2unlock = 0;
	break;
     default:
        break;
  } // State actions
  //PORTD = tmpDT1;
  return state;
}


int TickFct_OnOff(int state) {
  unsigned char tmpA = (~PINA & 0x02); 
  switch(state) { // Transitions
     case OnOff_SMStart: // Initial transition
        state = s_Off;
        break;
     case s_Off:
	count = 0;
	if (tmpA == 0x02) state = s_On;
	else state = s_Off;
        break;
     case s_On:
	if (count < MAXCOUNT-1) {
		count++;
		state = s_On;
	}
	else{
		count = 0;
		state = s_Off;
	}
        break;
     default:
        state = OnOff_SMStart;
	break;
   } // Transitions

  switch(state) { // State actions
     case OnOff_SMStart: // Initial transition
        break;
     case s_Off:
	set_PWM(0);
        break;
     case s_On:
	set_PWM(frequency[keySeq[count]]);
        break;
     default:
        break;
  } // State actions
  return state;
}



/*
int TickFct_Combined(int state) {
  switch(state) { // Transitions
     case C_SMStart: // Initial transition
        state = C_s1;
        break;
     case C_s1:
        state = C_s1;
        break;
     default:
        state = C_SMStart;
   } // Transitions

  switch(state) { // State actions
     case C_SMStart: // Initial transition
     	PORTD = tmpDT1 | tmpDT2 | tmpDT4; 
        break;
     case C_s1:
     	PORTD = tmpDT1 | tmpDT2 | tmpDT4; 
        break; 
     default:
     	PORTD = tmpDT1 | tmpDT2 | tmpDT4; 
        break;
  } // State actions
  return state;
}

int TickFct_Speaker(int state) {
	unsigned char tmpA = (~PINA & 0x04) >> 2; 
  switch(state) { // Transitions
     case SP_SMStart: // Initial transition
        tmpDT4 = 0; // Initialization behavior
        state = SP_s0;
        break;
     case SP_s0:
	if(tmpA == 0x01)
		state = SP_s1;
	else
		state = SP_s0;
     case SP_s1:
	if(tmpA == 0x01)
        	state = SP_s1;
	else
		state = SP_s0;
        break;
     default:
        state = SP_SMStart;
   } // Transitions

  switch(state) { // State actions
     case SP_s0:
        tmpDT4 = 0x00;
        break;
     case SP_s1:
        tmpDT4 ^= 0x10;
        break;
     default:
        break;
  } // State actions
  return state;
}

int TickFct_FRQ(int state) {
unsigned char tmpA = (~PINA & 0x03);
  switch(state) { // Transitions
     case FRQ_SMStart: // Initial transition
        state = FRQ_s0;
        break;
     case FRQ_s0:
	if(tmpA == 0x00)
		state = FRQ_s0;
	else if(tmpA == 0x01)
		state = FRQ_dec;
	else if(tmpA == 0x02)
		state = FRQ_inc;
	break;
     case FRQ_inc:
	state = FRQ_s0;
        break;
     case FRQ_dec:
	state = FRQ_s0;
        break;
     default:
        state = FRQ_SMStart;
   } // Transitions

  switch(state) { // State actions
     case FRQ_s0:
  	//tasks[3].period = 1;
        break;
     case FRQ_inc:
        periodSpeaker--;
  	tasks[3].period = periodSpeaker;
        break;
     case FRQ_dec:
        periodSpeaker++;
  	tasks[3].period = periodSpeaker;
        break;
     default:
        break;
  } // State actions
  return state;
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
*/
