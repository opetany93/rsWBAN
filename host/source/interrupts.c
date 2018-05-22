#include <math.h>
#include <stdio.h>

#include "nrf.h"

#include "radio.h"
#include "protocol_host.h"
#include "lcd_Nokia5110.h"
#include "mydefinitions.h"

// ===================================================================================
extern volatile uint8_t amountOfConnectedSensors;

volatile char buf_lcd[20];
int angle1 = 0;
int x1, y1;

static void lcd_SpinningLine(void);

volatile uint8_t slotCnt = 0;

// ===================================================================================
void RADIO_IRQHandler(void)
{
	if(RADIO->EVENTS_END)
	{
		RADIO->EVENTS_END = 0U;
		
		radioHostHandler();
	}
}

// ===================================================================================
void RTC0_IRQHandler()
{
	if (RTC0->EVENTS_COMPARE[0])					// SYNC stop and start time slots (via PPI)
	{
		RTC0->EVENTS_COMPARE[0] = 0;

		startTimeSlotListener();
	}

	if (RTC0->EVENTS_COMPARE[1])					// SYNC start
	{
		RTC0->EVENTS_COMPARE[1] = 0;

		syncTransmitHandler();
	}
}

// ===================================================================================
void RTC1_IRQHandler()								// time slot for sensor
{
	if (RTC1->EVENTS_COMPARE[0])
	{
		RTC1->EVENTS_COMPARE[0] = 0;
		
		timeSlotListenerHandler();

		if(slotCnt > 1)								// ostatni¹ szczelinê koñczy koniec ramki SUF, dlatego potrzebne jest zliczanie do 3, ¿eby nie wyskoczy³o przerwanie przed koñcem SUF
		{
			RTC1->TASKS_STOP = 1U;
			slotCnt = 0;
		}
		else
		{
			slotCnt++;
		}
	}
}

// =======================================================================================
void TIMER1_IRQHandler(void)						// odswie¿anie wyswietlacza
{
	if(TIMER1->EVENTS_COMPARE[0])
	{
		TIMER1->EVENTS_COMPARE[0] = 0U;
		
		TIMER1->TASKS_CLEAR = 1U;
		
		sprintf((char *)buf_lcd, "%d", amountOfConnectedSensors);
		lcd_draw_text(3, 50, (char *)buf_lcd);
		
		lcd_SpinningLine();
		
		lcd_copy();
	}
}

// ===================================================================================
static void lcd_SpinningLine(void)
{
	uint8_t i, k;
	
	for(k = 0; k < 8; k++)
	{
		for(i = 0; i < 8; i++)
		{
			lcd_clear_pixel(i, k);
		}
	}
	
	angle1 += 10;
	x1 = 4.0 * sin( angle1 * 3.14 / 180.0);
	y1 = 4.0 * cos( angle1 * 3.14 / 180.0);
		
	lcd_draw_line(4 + x1, 4 - y1, 4 - x1, 4 + y1);
}
