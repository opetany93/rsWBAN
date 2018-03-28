#include "hal.h"

#include "nrf_gpio.h"
#include "nrf.h"
#include "nrf_delay.h"

#include "clocks.h"

#include "mytypes.h"
#include "mydefinitions.h"

#include <stdio.h>
#include "uart.h"

void error(void)
{
	while(1)
	{
		LED_2_TOGGLE();
		nrf_delay_ms(1000);
	}
}

// -------------------------------------------------------------------------------------
#if defined(NRF52_SENSOR)
// -------------------------------------------------------------------------------------

#define DC_DC_CONVERTER_ON 		0

void boardInit(void)
{
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);			// preemption 3 bits, subpriority 1 bit
	
	nrf_gpio_cfg_output(LED_1);
	nrf_gpio_cfg_output(LED_2);
	nrf_gpio_cfg_output(SCL_PIN);
	
	nrf_gpio_pin_set(LED_1);
	nrf_gpio_pin_set(LED_2);

#if DC_DC_CONVERTER_ON	
	NRF_POWER->DCDCEN = 1U;															//Internal DC/DC regulator enable
#endif
	
	if ( 0 > startLFCLK() )					// start 32,768 kHz oscillator
	{
		error();
	}
	
	//set NVIC for TIMER0 which is used for timeout in function ReadPacketWithTimeout
	NVIC_SetPriority(TIMER0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), TIMER0_INTERRUPT_PRIORITY, 0));
	NVIC_EnableIRQ(TIMER0_IRQn);
}

//===================================================================================
void buttonInterruptInit(void)
{
	// button gpio init for interrupt 
	nrf_gpio_cfg_sense_input(BUTTON, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
	BUTTON_INTERRUPT_ENABLE();														// enable interrupt from PORT EVENT
	
	NVIC_SetPriority(GPIOTE_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), GPIOTE_INTERRUPT_PRIORITY, 0));									// set and enable NVIC for button interrupt
	NVIC_EnableIRQ(GPIOTE_IRQn);
}

// -------------------------------------------------------------------------------------
#elif defined(BOARD_PCA10040)
// -------------------------------------------------------------------------------------
char buf[150];

//===================================================================================
void boardInit(void)
{
	NVIC_SetPriorityGrouping(7 - PREEMPTION_PRIORITY_BITS);
	
	//systick - przerwanie co 1ms
	//SysTick_Config(SystemCoreClock/1000);
	
	nrf_gpio_cfg_output(LED_1);
	nrf_gpio_cfg_output(LED_2);
	nrf_gpio_cfg_output(LED_3);
	nrf_gpio_cfg_output(LED_4);
	
	nrf_gpio_cfg_output(31);	//debug
	nrf_gpio_cfg_output(ARDUINO_0_PIN);	//debug
	nrf_gpio_cfg_output(12);	//debug
	//nrf_gpio_cfg_output(SCL_PIN);
	
	nrf_gpio_pin_set(LED_1);
	nrf_gpio_pin_set(LED_2);
	nrf_gpio_pin_set(LED_3);
	nrf_gpio_pin_set(LED_4);
	
	//gpio init - buttons
	nrf_gpio_cfg_input(BUTTON_1, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(BUTTON_2, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(BUTTON_3, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(BUTTON_4, NRF_GPIO_PIN_PULLUP);
	
	uartInit();
	//uartDmaInit();
	//timInit();
	
	sprintf(buf,"\r\n========== nRF52xxx HOST ===========\r\n");
	uartWriteS(buf);
	sprintf(buf,"Compilation: %s,%s\r\n\r\n",__DATE__,__TIME__);
	uartWriteS(buf);
	sprintf(buf,"TEST WIRELESS PROTOCOL\r\n\r\n");
	uartWriteS(buf);

	//NVIC_SetPriority(RNG_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 10, 0));
	NVIC_EnableIRQ(RNG_IRQn);
	
	NRF_RNG->CONFIG = 1;
	
	if ( 0 > startLFCLK() )					// start 32,768 kHz oscillator
	{
		error();
	}
}
#endif
