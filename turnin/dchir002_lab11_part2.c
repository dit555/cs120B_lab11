/*	Author: Dumitru Chiriac lab 11
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
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "bit.h"
#include "keypad.h"
#include "io.h"
#endif


volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
        TCCR1B = 0x0B;
        OCR1A = 125;
        TIMSK1 = 0x02;
        TCNT1 = 0;

        _avr_timer_cntcurr = _avr_timer_M;
        SREG |= 0x80;

}

void TimerISR() {
        TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
        _avr_timer_cntcurr--;
        if(_avr_timer_cntcurr == 0) {
                TimerISR();
                _avr_timer_cntcurr = _avr_timer_M;
        }
}


void TimerSet(unsigned long M){
        _avr_timer_M = M;
        _avr_timer_cntcurr = _avr_timer_M;
}

typedef struct task {
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int (*TickFct)(int);
} task;

task tasks[2];
const unsigned char tasksNum = 2;
const unsigned long pG = 100;
const unsigned long p1 = 100;
const unsigned long p2 = 300;

//variables for CSM
unsigned char x;
unsigned char temp;
const unsigned char m1[] = "CS120B is Legend";
const unsigned char m2[] = "... wait for it ";
const unsigned char m3[] = "DARY!           ";

//sms
enum Lights { L_Start, L_disp};
int Tick_L(int state);

enum Message {M_Start, M_Wait, M_1, M_2, M_3};
int Tick_M(int state);

void RUN() { //changed name from timerISR to avoid conflictiong things
        unsigned char i;
        for (i = 0; i < tasksNum; i++){
                if (tasks[i].elapsedTime >= tasks[i].period){
                        while (!TimerFlag);
                        tasks[i].state = tasks[i].TickFct(tasks[i].state);
                        tasks[i].elapsedTime = 0;
                        TimerFlag = 0;
                }
                tasks[i].elapsedTime += pG;
        }
}

int main(void) {
	
	unsigned char i = 0;
        //task 1
        tasks[i].state = L_Start;
        tasks[i].period = p1;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &Tick_L;
        i++;

	//task 2
        tasks[i].state = M_Start;
        tasks[i].period = p2;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &Tick_M;
        i++;

	
	TimerSet(pG);
	TimerOn();

	DDRA = 0xFF; PORTA = 0x00;
     	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	LCD_init();
	LCD_DisplayString(1, m1);
    	while (1) {
		RUN();
		
    	}
    	return 1;
}

int Tick_L(int state){

	switch(state){
		case L_Start: state = L_disp; break;
		case L_disp: state = L_disp; break;
		default: state = L_Start; break;
	}

	switch(state){
		case L_disp:
			x = GetKeypadKey();
  		        switch(x){
                        	case '\0': PORTB = 0x1F; break;
                        	case '1': PORTB = 0x01; temp = 1; break;
	                        case '2': PORTB = 0x02; temp = 2; break;
       		                case '3': PORTB = 0x03; temp = 3;break;
                        	case '4': PORTB = 0x04; break;
                        	case '5': PORTB = 0x05; break;
                        	case '6': PORTB = 0x06; break;
                        	case '7': PORTB = 0x07; break;
                        	case '8': PORTB = 0x08; break;
                        	case '9': PORTB = 0x09; break;
                        	case 'A': PORTB = 0x0A; break;
                        	case 'B': PORTB = 0x0B; break;
                        	case 'C': PORTB = 0x0C; break;
                        	case 'D': PORTB = 0x0D; break;
                        	case '*': PORTB = 0x0E; break;
                        	case '0': PORTB = 0x00; break;
                        	case '#': PORTB = 0x0F; break;
                        	default: PORTB = 0x1B; break;
                	}
			break;
		default: break;

	}
	return state;
}

int Tick_M(int state){
	switch(state){
		case M_Start: state = M_Wait; break;
		case M_Wait:
			if (temp == 1)
				state = M_1;
			else if(temp == 2)
				state = M_2;
			else if(temp == 3)
				state = M_3;
			else
				state = M_Wait;
			break;
		case M_1: state = M_Wait; break;
		case M_2: state = M_Wait; break;
		case M_3: state = M_Wait; break;
		default: state = M_Start;
	}

	switch(state){
		case M_1: LCD_ClearScreen(); LCD_DisplayString(1, m1); break;
		case M_2: LCD_ClearScreen(); LCD_DisplayString(1, m2); break;
		case M_3: LCD_ClearScreen(); LCD_DisplayString(1, m3); break;

	}
	return state;
}
