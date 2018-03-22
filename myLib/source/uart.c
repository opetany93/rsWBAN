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

	UARTE->ENABLE = UARTE_ENABLE_ENABLE_Enabled;
	UARTE->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud1M;

	UARTE->TXD.MAXCNT = 0; //FTPAN109
	UARTE->INTENSET = UARTE_INTENSET_TXSTARTED_Msk; //FTPAN109
	UARTE->INTENSET = UARTE_INTENSET_ENDTX_Msk; //FTPAN109
	NVIC_SetPriority(UARTE0_UART0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
										 LOW_IRQ_PRIO, UARTE_IRQ_PRIORITY));
	NVIC_EnableIRQ(UARTE0_UART0_IRQn);

	UARTE->PSEL.RXD = UART_RX_PIN;
	UARTE->PSEL.TXD = UART_TX_PIN;
	UARTE->PSEL.CTS = NRF_UARTE_PSEL_DISCONNECTED;
	UARTE->PSEL.RTS = NRF_UARTE_PSEL_DISCONNECTED;
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
	
	//Enable UART
	UART->ENABLE = UART_ENABLE_ENABLE_Enabled;
	//Start UART transmitter
	UART->TASKS_STARTTX = 1U;
	//Start UART receiver
	UART->TASKS_STARTRX = 1U;
	
	//set speed
	UART->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud1M;
	//set tx pin
	UART->PSELRXD = UART_RX_PIN;
	UART->PSELTXD = UART_TX_PIN;
	
	//disable RTS and CTS
	UART->PSELRTS = NRF_UART_PSEL_DISCONNECTED;
	UART->PSELCTS = NRF_UART_PSEL_DISCONNECTED;
}

//===============================================================================================
void setUartIrqFunc(void (*pFunc)(char data))
{
	if(pFunc)
	{
		uartRE = pFunc;
		
		//NVIC set priority for interrupt
		NVIC_SetPriority(UARTE0_UART0_IRQn, 0x09);
		//enable interrupt in NVIC
		NVIC_EnableIRQ (UARTE0_UART0_IRQn);
		//enable interrupt from receiver
		UART->INTENSET = (UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos);
	}
	else
	{
		//disable interrupt from receiver
		UART->INTENCLR = (UART_INTENCLR_RXDRDY_Enabled << UART_INTENCLR_RXDRDY_Pos);
		//disable interrupt in NVIC
		NVIC_DisableIRQ(UARTE0_UART0_IRQn);
		
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
		if (UARTE->EVENTS_ENDTX)
		{
			UARTE->EVENTS_ENDTX = 0;

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
