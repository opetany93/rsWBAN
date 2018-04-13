#include <math.h>
#include <stdio.h>

#include "radio.h"
#include "protocol_host.h"
#include "lcd_Nokia5110.h"
#include "mydefinitions.h"

// ===================================================================================
extern volatile uint8_t connected_sensors_amount;

volatile char buf_lcd[20];
int angle1 = 0;
int x1, y1;

void lcd_SpinningLine(void);

volatile uint8_t temp = 0;

// ===================================================================================
void RADIO_IRQHandler(void)
{
	if(RADIO->EVENTS_END)
	{
		RADIO->EVENTS_END = 0U;
		
		//radioHostHandler();
	}
}

// ===================================================================================
void RTC0_IRQHandler()
{
	if (RTC0->EVENTS_COMPARE[0])					// SYNC stop and start time slots (via PPI)
	{
		RTC0->EVENTS_COMPARE[0] = 0;

		NRF_GPIO->OUTSET = (1 << ARDUINO_1_PIN);
		NRF_GPIO->OUTCLR = (1 << ARDUINO_1_PIN);
	}

	if (RTC0->EVENTS_COMPARE[1])					// SYNC start
	{
		RTC0->EVENTS_COMPARE[1] = 0;

		NRF_GPIO->OUTSET = (1 << ARDUINO_0_PIN);
		NRF_GPIO->OUTCLR = (1 << ARDUINO_0_PIN);

		//syncTransmitHandler();
	}
}

// ===================================================================================
void RTC1_IRQHandler()								// time slot for sensor
{
	if (RTC1->EVENTS_COMPARE[0])
	{
		RTC1->EVENTS_COMPARE[0] = 0;
		
		if ((temp >= 0) && (temp < 3))
		{
			NRF_GPIO->OUTSET = (1 << 31);
			NRF_GPIO->OUTCLR = (1 << 31);

			//timeSlotListenerHandler();
		}

		if(temp > 1)
		{
			RTC1->TASKS_STOP = 1U;
			temp = 0;
		}
		else
		{
			temp++;
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
		
		sprintf((char *)buf_lcd, "%d", connected_sensors_amount);
		lcd_draw_text(3, 50, (char *)buf_lcd);
		
		lcd_SpinningLine();
		
		lcd_copy();
	}
}

// ===================================================================================
void lcd_SpinningLine(void)
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
