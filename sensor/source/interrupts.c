#include "radio.h"
#include "protocol.h"
#include "LPS22HB.h"
#include "spi.h"
#include "mydefinitions.h"
#include "ADXL362.h"

#include "nrf_gpio.h"
#include "nrf_delay.h"

#define ADXL362_ENABLE 0

#if ADXL362_ENABLE
spiHandle_t adxl362spiHandle;
#endif

//===================================================================================
void RADIO_IRQHandler(void)
{
	if(RADIO->EVENTS_END)
	{
		RADIO->EVENTS_END = 0U;
		
		radioSensorHandler();
	}
}

// =======================================================================================
void RTC0_IRQHandler()
{
  if (RTC0->EVENTS_COMPARE[0])
  {
		RTC0->EVENTS_COMPARE[0] = 0U;
		
		timeSlotHandler();
  }
	
  if (RTC0->EVENTS_COMPARE[1])
  {
	  RTC0->EVENTS_COMPARE[1] = 0U;
		
	  syncHandler();
  }
}

#if ADXL362_ENABLE
static void spiSendADXL362Interface(uint8_t byte){ spiSend(adxl362spiHandle, byte); }
static uint8_t spiReadADXL362Interface(void){ return spiRead(adxl362spiHandle); }
static void spiControlChipSelectADXL362Interface(uint8_t state){ spiControlChipSelect(adxl362spiHandle, state); }
#endif

// =======================================================================================
void GPIOTE_IRQHandler(void)
{
	if(GPIOTE->EVENTS_PORT)
	{
		GPIOTE->EVENTS_PORT = 0U;
		
		//LED_2_TOGGLE();

		connect_status_t status = connect();

		if( CONNECTED == status )
		{
#if ADXL362_ENABLE
			adxl362spiHandle = spiInit(SPI0, SCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN, SPI_FREQUENCY_FREQUENCY_M4, SPI_MODE_0, SPI_ORDER_MSB_FIRST);
			if ( 0 > ADXL362_Init(spiSendADXL362Interface, spiReadADXL362Interface, spiControlChipSelectADXL362Interface) )
			{
				error();
			}
#endif
			
#if FIFO_ENABLED

			if (0 > ADXL362_FifoInit(5) )
			{
				error();
			}
#endif

			//BUTTON_INTERRUPT_DISABLE();
		}
		else if(ALREADY_CONNECTED == status)
		{
			//set NVIC for TIMER1 which is used for timeout in function ReadPacketWithTimeout
			NVIC_SetPriority(TIMER1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), LOW_IRQ_PRIO, TIMER1_INTERRUPT_PRIORITY));
			NVIC_EnableIRQ(TIMER1_IRQn);

			//24bit mode
			TIMER1->BITMODE = TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos;
			//Enable interrupt for COMPARE[0]
			TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Enabled <<  TIMER_INTENSET_COMPARE0_Pos;

			TIMER1->TASKS_STOP = 1U;
			TIMER1->TASKS_CLEAR = 1U;

			TIMER1->CC[0] = 2000 * 1000;
			TIMER1->TASKS_START = 1U;
		}
	}
}

// =======================================================================================
void TIMER0_IRQHandler(void)
{
	if(TIMER0->EVENTS_COMPARE[0])
	{
		TIMER0->EVENTS_COMPARE[0] = 0U;
		
		timeoutInterruptHandler();
	}
}

// =======================================================================================
void TIMER1_IRQHandler(void)
{
	if(TIMER1->EVENTS_COMPARE[0])
	{
		TIMER1->EVENTS_COMPARE[0] = 0U;

		TIMER1->TASKS_STOP = 1U;

		if(0 == nrf_gpio_pin_read(BUTTON))
		{
			deInitProtocol();
			SYSTEM_OFF();
		}
	}
}
