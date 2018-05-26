#include "radio.h"
#include "../../protocol/inc/protocol.h"
#include "LPS22HB.h"
#include "spi.h"
#include "mydefinitions.h"
#include "ADXL362.h"

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_rtc.h"
#include "nrf_gpiote.h"
#include "nrf_timer.h"

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
  if (nrf_rtc_event_pending(RTC0,NRF_RTC_EVENT_COMPARE_0))
  {
		nrf_rtc_event_clear(RTC0, NRF_RTC_EVENT_COMPARE_0);
		timeSlotHandler();
  }
	
  if (nrf_rtc_event_pending(RTC0,NRF_RTC_EVENT_COMPARE_1))
  {
	  nrf_rtc_event_clear(RTC0, NRF_RTC_EVENT_COMPARE_1);
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
	if(nrf_gpiote_event_is_set(NRF_GPIOTE_EVENTS_IN_0))
	{
		nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_IN_0);

		protocol_status_t status = connect();

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

			nrf_timer_bit_width_set(TIMER1, NRF_TIMER_BIT_WIDTH_24);
			nrf_timer_int_enable(TIMER1, TIMER_INTENSET_COMPARE0_Msk);
			nrf_timer_task_trigger(TIMER1, NRF_TIMER_TASK_STOP);
			nrf_timer_task_trigger(TIMER1, NRF_TIMER_TASK_CLEAR);
			nrf_timer_cc_write(TIMER1, NRF_TIMER_CC_CHANNEL0, 2000 * 1000);
			nrf_timer_task_trigger(TIMER1, NRF_TIMER_TASK_START);

			// TODO: opakuj powy¿sze ustawienie timera w funkcjê
		}
	}
}

// =======================================================================================
void TIMER0_IRQHandler(void)
{
	if(nrf_timer_event_check(TIMER0, NRF_TIMER_EVENT_COMPARE0))
	{
		nrf_timer_event_clear(TIMER0, NRF_TIMER_EVENT_COMPARE0);
		timeoutInterruptHandler();
	}
}

// =======================================================================================
void TIMER1_IRQHandler(void)
{
	if(nrf_timer_event_check(TIMER1, NRF_TIMER_EVENT_COMPARE0))
	{
		nrf_timer_event_clear(TIMER1, NRF_TIMER_EVENT_COMPARE0);
		nrf_timer_task_trigger(TIMER1, NRF_TIMER_TASK_SHUTDOWN);

		if(0 == nrf_gpio_pin_read(BUTTON))
		{
			deInitProtocol();
		}
	}
}
