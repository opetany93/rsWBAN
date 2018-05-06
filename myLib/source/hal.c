#include "hal.h"

#include "nrf_gpio.h"
#include "nrf.h"
#include "nrf_delay.h"

#include "clocks.h"

#include "mytypes.h"
#include "mydefinitions.h"

#include <stdio.h>
#include "uart.h"

#define NRF52_ONRAM1_OFFRAM0    POWER_RAM_POWER_S0POWER_On 	    << POWER_RAM_POWER_S0POWER_Pos      \
							  | POWER_RAM_POWER_S1POWER_On      << POWER_RAM_POWER_S1POWER_Pos      \
							  | POWER_RAM_POWER_S0RETENTION_Off << POWER_RAM_POWER_S0RETENTION_Pos  \
	                          | POWER_RAM_POWER_S1RETENTION_Off << POWER_RAM_POWER_S1RETENTION_Pos;

void error(void)
{
	while(1)
	{
		LED_2_TOGGLE();
		nrf_delay_ms(1000);
	}
}

static void initGpio()
{
#if defined(NRF52_SENSOR)
	nrf_gpio_cfg_output(LED_1);
	nrf_gpio_cfg_output(LED_2);
	nrf_gpio_cfg_output(SCL_PIN);

	nrf_gpio_pin_set(LED_1);
	nrf_gpio_pin_set(LED_2);

#elif defined(BOARD_PCA10040)
	nrf_gpio_cfg_output(LED_1);
	nrf_gpio_cfg_output(LED_2);
	nrf_gpio_cfg_output(LED_3);
	nrf_gpio_cfg_output(LED_4);

	nrf_gpio_cfg_output(ARDUINO_0_PIN);
	nrf_gpio_cfg_output(ARDUINO_1_PIN);
	nrf_gpio_cfg_output(31);

	nrf_gpio_pin_set(LED_1);
	nrf_gpio_pin_set(LED_2);
	nrf_gpio_pin_set(LED_3);
	nrf_gpio_pin_set(LED_4);

	//buttons
	nrf_gpio_cfg_input(BUTTON_1, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(BUTTON_2, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(BUTTON_3, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(BUTTON_4, NRF_GPIO_PIN_PULLUP);

	nrf_gpio_cfg_output(ARDUINO_1_PIN); // for measurement
#endif
}

// -------------------------------------------------------------------------------------
#if defined(NRF52_SENSOR)
// -------------------------------------------------------------------------------------

#define DC_DC_CONVERTER_ON 		1

void boardInit(void)
{
	NVIC_SetPriorityGrouping(7 - PREEMPTION_PRIORITY_BITS);
	
	initGpio();

#if DC_DC_CONVERTER_ON	
	NRF_POWER->DCDCEN = 1U;															//Internal DC/DC regulator enable
#endif
	
	if ( 0 > startLFCLK() )					// start 32,768 kHz oscillator
	{
		error();
	}
	
//	NRF_POWER->RAM[0].POWER = NRF52_ONRAM1_OFFRAM0;
//	NRF_POWER->RAM[1].POWER = NRF52_ONRAM1_OFFRAM0;
//	NRF_POWER->RAM[2].POWER = NRF52_ONRAM1_OFFRAM0;
//	NRF_POWER->RAM[3].POWER = NRF52_ONRAM1_OFFRAM0;
//	NRF_POWER->RAM[4].POWER = NRF52_ONRAM1_OFFRAM0;
//	NRF_POWER->RAM[5].POWER = NRF52_ONRAM1_OFFRAM0;
//	NRF_POWER->RAM[6].POWER = NRF52_ONRAM1_OFFRAM0;
//	NRF_POWER->RAM[7].POWER = NRF52_ONRAM1_OFFRAM0;
//
//	NRF_POWER->RESETREAS = POWER_RESETREAS_OFF_Msk;

}

//===================================================================================
void buttonInterruptInit(void)
{
	// button gpio init for interrupt 
	nrf_gpio_cfg_sense_input(BUTTON, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
	BUTTON_INTERRUPT_ENABLE();										// enable interrupt from PORT EVENT
	
	NVIC_SetPriority(GPIOTE_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), LOW_IRQ_PRIO, GPIOTE_INTERRUPT_PRIORITY));									// set and enable NVIC for button interrupt
	NVIC_EnableIRQ(GPIOTE_IRQn);
}

// -------------------------------------------------------------------------------------
#elif defined(BOARD_PCA10040)
// -------------------------------------------------------------------------------------
char uartBuf[100];

//===================================================================================
void boardInit(void)
{
	NVIC_SetPriorityGrouping(7 - PREEMPTION_PRIORITY_BITS);

	//systick - przerwanie co 1ms
	//SysTick_Config(SystemCoreClock/1000);

	initGpio();
	
	uartInit();
	sprintf(uartBuf,"\r\n========== nRF52xxx HOST ===========\r\n");
	uartWriteS(uartBuf);
	sprintf(uartBuf,"Compilation: %s,%s\r\n\r\n",__DATE__,__TIME__);
	uartWriteS(uartBuf);
	sprintf(uartBuf,"Real-Time Synchronous WBAN\r\n\r\n");
	uartWriteS(uartBuf);
	
	if ( 0 > startLFCLK() )					// start 32,768 kHz oscillator
	{
		error();
	}
}
#endif

inline void gpioGeneratePulse(uint8_t pin)
{
	NRF_GPIO->OUTSET = (1 << pin);
	NRF_GPIO->OUTCLR = (1 << pin);
}

inline void sleep()
{
	__WFE();						// wait for event, sleep mode
}
