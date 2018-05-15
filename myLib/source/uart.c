#include "uart.h"

#include <string.h>

#include "hal.h"
#include "nrf_uart.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_uarte.h"
#include "mydefinitions.h"

volatile uint8_t temp;
static void (*uartRE)(char data);

volatile uint32_t dmaLength;

void uartDmaInit(void)
{
	//gpio config TX
	NRF_GPIO ->PIN_CNF[UART_TX_PIN] = ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
									|  (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos)
									|  (GPIO_PIN_CNF_PULL_Pulldown  << GPIO_PIN_CNF_PULL_Pos)
									|  (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos));

	//gpio config RX
	NRF_GPIO ->PIN_CNF[UART_RX_PIN] = ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
									|  (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos)
									|  (GPIO_PIN_CNF_PULL_Disabled  << GPIO_PIN_CNF_PULL_Pos)
									|  (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos));

	nrf_uarte_enable(UARTE);
	nrf_uarte_baudrate_set(UARTE, NRF_UARTE_BAUDRATE_1000000);

	UARTE->TXD.MAXCNT = 0; //FTPAN109
	UARTE->INTENSET = UARTE_INTENSET_TXSTARTED_Msk; //FTPAN109
	UARTE->INTENSET = UARTE_INTENSET_ENDTX_Msk; //FTPAN109
	NVIC_SetPriority(UARTE0_UART0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
										 LOW_IRQ_PRIO, UARTE_IRQ_PRIORITY));
	NVIC_EnableIRQ(UARTE0_UART0_IRQn);

	nrf_uarte_txrx_pins_set(UARTE, UART_TX_PIN, UART_RX_PIN);
	nrf_uarte_hwfc_pins_disconnect(UARTE);
}

//===============================================================================================
void uartInit(void)
{
	//gpio config TX
	NRF_GPIO ->PIN_CNF[UART_TX_PIN] = ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
									|  (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos)
									|  (GPIO_PIN_CNF_PULL_Pulldown  << GPIO_PIN_CNF_PULL_Pos)
									|  (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos));
	
	//gpio config RX
	NRF_GPIO ->PIN_CNF[UART_RX_PIN] = ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
									|  (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos)
									|  (GPIO_PIN_CNF_PULL_Disabled  << GPIO_PIN_CNF_PULL_Pos)
									|  (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos));
	
	
	nrf_uart_enable(UART);										//Enable UART
	nrf_uart_task_trigger(UART, NRF_UART_TASK_STARTTX);			//Start UART transmitter
	nrf_uart_task_trigger(UART, NRF_UART_TASK_STARTRX);			//Start UART receiver
	nrf_uart_baudrate_set(UART, NRF_UARTE_BAUDRATE_1000000);	//set speed

	nrf_uart_txrx_pins_set(UART, UART_TX_PIN, UART_RX_PIN);
	nrf_uart_hwfc_pins_disconnect(UART);					//disable RTS and CTS
}

//===============================================================================================
void setUartIrqFunc(void (*pFunc)(char data))
{
	if(pFunc)
	{
		uartRE = pFunc;
		
		//NVIC set priority for interrupt
		NVIC_SetPriority(UARTE0_UART0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
											LOW_IRQ_PRIO, UARTE_IRQ_PRIORITY));
		NVIC_EnableIRQ (UARTE0_UART0_IRQn);								//enable interrupt in NVIC
		nrf_uarte_int_enable(UARTE, UARTE_INTENSET_RXDRDY_Msk);			//enable interrupt from receiver
	}
	else
	{
		nrf_uart_int_disable(UART, UART_INTENCLR_RXDRDY_Msk);			//disable interrupt from receiver
		NVIC_DisableIRQ(UARTE0_UART0_IRQn);								//disable interrupt in NVIC
		
		uartRE = NULL;
	}
}

//===============================================================================================
void UARTE0_UART0_IRQHandler(void)
{
	volatile int dummy;

	if (UARTE->EVENTS_TXSTARTED)
	{
		UARTE->EVENTS_TXSTARTED = 0;

		if (UARTE->TXD.MAXCNT == 0)
		{
			UARTE->TXD.MAXCNT = dmaLength;
			UARTE->TASKS_STARTTX = 1;
		}
	}
	else
	{
		if (nrf_uarte_event_check(UARTE, NRF_UARTE_EVENT_ENDTX))
		{
			nrf_uarte_event_clear(UARTE, NRF_UARTE_EVENT_ENDTX);

			if (UARTE->TXD.MAXCNT != 0)
			{
				UARTE->TXD.MAXCNT = 0;
			}
		}
	}

	// for tx
	if(nrf_uart_event_check(NRF_UART0, NRF_UART_EVENT_TXDRDY))
  {
	  UART->EVENTS_TXDRDY = 0x0;
	}
	
	// for rx
	if (nrf_uart_event_check(NRF_UART0, NRF_UART_EVENT_RXDRDY))
  {
		UART->EVENTS_RXDRDY = 0x0;
		temp = UART->RXD;
		uartRE(temp);
	}
	
	dummy=UART->EVENTS_RXDRDY;				// bez tego tez dziala ale lepiej zeby bylo zgodnie z rekomendacja Nordic
}

//===============================================================================================
void uartWrite(char data)
{
	UART->TXD = data;

	while(!(UART->EVENTS_TXDRDY));			//czekaj dopóki nie zostanie wyslane, EVENTS_TXDRDY == 1 - wsyslane
	UART->EVENTS_TXDRDY = 0x0;				//czysci flage
}

//===============================================================================================
void uartWriteS(char *s)
{
	while(*s) uartWrite(*s++);
}

void uartDmaWriteS(char *s, uint32_t length)
{
	UARTE->TXD.PTR = (uint32_t)s;
	dmaLength = length;

	UARTE->TASKS_STARTTX = 1U;
}
