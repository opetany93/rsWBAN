#include "protocol.h"
#include "radio.h"
#include "spi.h"
#include "clocks.h"
#include "nrf_gpio.h"
#include "mydefinitions.h"
#include "ADXL362.h"
#include "mytypes.h"

#define ATTEMPTS_OF_CONNECT		20

char 							packet[PACKET_SIZE];
volatile uint8_t 				channel, radio_rx_status;
volatile uint8_t				packetLength = 24;

//=======================================================================================
static void PPI_Init()
{
	// connect RTC0 COMPARE[0] EVENT to CLOCK HFCLKSTART TASK
	PPI->CH[0].EEP = (uint32_t) &NRF_RTC0->EVENTS_COMPARE[0];
	PPI->CH[0].TEP = (uint32_t) &NRF_CLOCK->TASKS_HFCLKSTART;
	PPI->CHENSET = PPI_CHENSET_CH0_Enabled << PPI_CHENSET_CH0_Pos;

	// connect RADIO DISABLED EVENT to CLOCK TASK HFCLKSTOP
	PPI->CH[1].EEP = (uint32_t) &NRF_RADIO->EVENTS_DISABLED;
	PPI->CH[1].TEP = (uint32_t) &NRF_CLOCK->TASKS_HFCLKSTOP;
	PPI->CHENSET = PPI_CHENSET_CH1_Enabled << PPI_CHENSET_CH1_Pos;
}

//=======================================================================================
static void RTC0_Init()
{
	RTC0->PRESCALER = 0;
	RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos; 	// enable generate event for compare0
	RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
	RTC0->INTENSET = RTC_INTENSET_COMPARE1_Enabled << RTC_INTENSET_COMPARE1_Pos;

	NVIC_SetPriority(RTC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), RTC0_INTERRUPT_PRIORITY, 0));
	NVIC_EnableIRQ(RTC0_IRQn);
}

//=======================================================================================
static uint8_t waitForSync()
{
	RADIO->FREQUENCY = SYNC_CHANNEL;

	if(RADIO_OK == readPacketWithTimeout((uint32_t *)packet, 200) )
	{
		return (SYNC == ((sync_packet_t *)packet)->sync);
	}
	else
	{
		return FALSE;
	}
}

// =======================================================================================
int8_t connect(void)
{
	uint8_t read_success = 3;
	
	startHFCLK();
	((data_packet_t *)packet)->payloadSize = 1;
	((data_packet_t *)packet)->type = PACKET_init;
	
	RADIO->FREQUENCY = ADVERTISEMENT_CHANNEL; 						// Frequency bin 30, 2430MHz
	
	for(uint8_t i = 0; (i < ATTEMPTS_OF_CONNECT) && (0 !=  read_success); i++)
	{
		read_success =  sendPacketWithResponse((uint32_t *)packet, 5);
		disableRadio();
		
		LED_2_TOGGLE();
	}
	
	if(0 == read_success)
	{
		channel = ((init_packet_t *)packet)->channel;
		
		RTC0->CC[0] = ((init_packet_t *)packet)->rtc_val_CC0;
		RTC0->CC[1] = ((init_packet_t *)packet)->rtc_val_CC1;

		PPI_Init();
		RTC0_Init();

		if(waitForSync())
		{
			RADIO->FREQUENCY = channel;
			RTC0->TASKS_CLEAR = 1U;
			RTC0->TASKS_START = 1U;
			RADIO->PACKETPTR = (uint32_t)packet;
			RADIO_END_INT_ENABLE();
		}
	}	
	
	return read_success;
}

//=======================================================================================
void radioSensorHandler()
{
	if(RADIO->STATE == RADIO_STATE_STATE_RxIdle)
	{
		if(RADIO->CRCSTATUS == 1U)
		{
			if( SYNC == ((sync_packet_t *)packet)->sync )
			{
				nrf_gpio_pin_set(SCL_PIN);
				//RTC0->CC[0] = init_packet.rtc_val_CC0;
				//RTC0->CC[1] = ack_packet.rtc_val_CC1;
				
				RTC0->TASKS_CLEAR = 1U;
				
				RADIO->FREQUENCY = channel;	
				
				radio_rx_status = RADIO_OK;
				nrf_gpio_pin_clear(SCL_PIN);
			}
			else
			{
				radio_rx_status = RADIO_ACKError;
				RTC0->TASKS_CLEAR = 1U;										// sync, but data was not properly readed
			}
		}
		else
		{
			RTC0->TASKS_CLEAR = 1U;										// sync, but data was not properly readed
			radio_rx_status = RADIO_CRCError;
		}
	}
	RADIO_DISABLE();
}

void timeSlotHandler()
{
	RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk;
	
	((data_packet_t *)packet)->payloadSize = packetLength - 1;
	((data_packet_t *)packet)->type = PACKET_data;
	
	SPI_ENABLE(SPI0);
	
#if FIFO_ENABLED
	ADXL362_ReadFifo(((data_packet_t *)packet)->data, 15);		// 5 samples of 3 axis
#else
	ADXL362_ReadXYZ(&((data_packet_t *)packet)->axes);
#endif

	SPI_DISABLE(SPI0);

	while ( !isHFCLKstable() );					// wait for stable HCLK which has been started via RTC0 event through PPI

	RADIO->TASKS_TXEN = 1U;
}

void syncHandler()
{
	RADIO->FREQUENCY = SYNC_CHANNEL;
	RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk;
	
	if ( 0 > startHFCLK() )
	{
		error();
	}
	
	RADIO->TASKS_RXEN = 1U;
	
	LED_2_TOGGLE();
}

