#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "hal.h"
#include "test.h"
#include "uart.h"
#include "lcd_Nokia5110.h"

#include <math.h>

#define PWM_CH0_DUTY 50
#define PWM_CH1_DUTY 100

float temperature; 
char temperatureBuf[10];


// ================================ variables for PWM ============================================
volatile uint16_t pwm_seq;
uint8_t up = 1;
// ===============================================================================================


//======================================================================================
void echoOnOff(void)
{
	//button 1 echo on
	if (!(nrf_gpio_pin_read(BUTTON_1)))
	{
		uartWriteS("\r\nButton1: teraz juz bez echa\r\n");
		setUartIrqFunc(0);

		nrf_delay_ms(300);
	}

	//button 2 echo off
	if (!(nrf_gpio_pin_read(BUTTON_2)))
	{
		uartWriteS("\r\nButton2: teraz z echem\r\n");
		setUartIrqFunc(uartWrite);
		
		nrf_delay_ms(300);
	}
}


//======================================================================================
void temperatureCore(void)
{
	//button 3 temperatura
	if (!(nrf_gpio_pin_read(BUTTON_3)))
	{
		NRF_TEMP ->TASKS_START = 1;

		if(NRF_TEMP ->EVENTS_DATARDY)
		{
			NRF_TEMP ->EVENTS_DATARDY = 0x0;

			temperature = NRF_TEMP ->TEMP/4.0;

			sprintf(temperatureBuf, "Actual core temperature: %.2f\n\r", temperature);
			uartWriteS(temperatureBuf);

			nrf_delay_ms(300);
		}
	}
}

//======================================= PWM ==========================================
void pwmInit(void)
{
	//pin select
	NRF_PWM0->PSEL.OUT[0] = (LED_1 << PWM_PSEL_OUT_PIN_Pos) 
												| (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
	
	NRF_PWM0->PSEL.OUT[1] = (LED_3 << PWM_PSEL_OUT_PIN_Pos) 
												| (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
	
	//PWM enable
	NRF_PWM0->ENABLE = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);

	//PWM mode - cunter count up
	NRF_PWM0 ->MODE = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
	
	//prescaler 1
	NRF_PWM0->PRESCALER = (PWM_PRESCALER_PRESCALER_DIV_1 << PWM_PRESCALER_PRESCALER_Pos);
	
	//Value up to which the pulse generator counter counts
	NRF_PWM0->COUNTERTOP = 1000; 
	
	NRF_PWM0->LOOP = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);
	
	NRF_PWM0->DECODER = (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos) |
											(PWM_DECODER_MODE_NextStep << PWM_DECODER_MODE_Pos);
	
	NRF_PWM0->SEQ[0].PTR = (uint32_t)&pwm_seq;
	NRF_PWM0->SEQ[0].CNT = 1;
	NRF_PWM0->SEQ[0].REFRESH = 0;
	NRF_PWM0->SEQ[0].ENDDELAY = 0;
	
	NRF_PWM0->TASKS_SEQSTART[0] = 1;
}

// ==========================================================================================
void pulse_diodes(void)
{
	if ( up == 1 ) pwm_seq++;
	if ( up == 0 ) pwm_seq--;
	if ( pwm_seq == 1000) up = 0;
	if ( pwm_seq == 0 ) 	 up = 1;
		
	NRF_PWM0->TASKS_SEQSTART[0] = 1;
	
	nrf_delay_ms(1);
}
