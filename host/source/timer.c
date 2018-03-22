#include "timer.h"
#include "hal.h"
#include "nrf.h"
#include "mydefinitions.h"

//===================================================================================
void timInit(void)
{
	//24bit mode
	TIMER0 ->BITMODE = (1<<1);
	
	//set prescaler 0, clock timer 16MHz
	//TIMER0 ->PRESCALER = 0x0;
	
	//Enable interrupt for COMPARE[0]
	TIMER0 ->INTENSET = (1<<16);
	
	//compare value, timer taktowany 1MHz, dziele przez wartosc 1000000 i otrzymuje przerwanie co 1s
	TIMER0 ->CC[0] = 1000000;
	
	//Start Timer
	TIMER0 ->TASKS_START = 0x1;
	
	//set NVIC
	NVIC_SetPriority(TIMER0_IRQn, 0x10);
	NVIC_EnableIRQ(TIMER0_IRQn);
}
