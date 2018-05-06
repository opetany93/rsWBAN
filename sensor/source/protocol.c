#include "protocol.h"
#include "radio.h"
#include "clocks.h"
#include "nrf_gpio.h"
#include "mydefinitions.h"
#include "mytypes.h"

#include <stddef.h>

#define ATTEMPTS_OF_CONNECT		100

char packet[PACKET_SIZE];
volatile uint8_t channel, radioRxStatus;
volatile uint8_t packetLength = 24;

// internal functions
static void PPI_Init();
static void RTC0_Init(uint32_t valueOfCC0, uint32_t valueOfCC1);
static uint8_t waitForSync(uint16_t ms);

static volatile uint8_t syncFlag, connectedFlag = 0;

Radio *radio = NULL;

__WEAK void timeSlotCallback()
{

}

//=======================================================================================
void initProtocol(Radio *radioDrv)
{
	radio = radioDrv;
}

//=======================================================================================
void deInitProtocol()
{
	connectedFlag = 0;
}

// =======================================================================================
connect_status_t connect(void)
{
	connect_status_t connectStatus = DISCONNECTED;
	RADIO_status_t responseStatus = 4;

	if(!connectedFlag)
	{
		startHFCLK();
		((data_packet_t *)packet)->payloadSize = 1;
		((data_packet_t *)packet)->packetType = PACKET_init;

		radio->setChannel(ADVERTISEMENT_CHANNEL); 						// Frequency bin 30, 2430MHz

		for(uint8_t i = 0; (i < ATTEMPTS_OF_CONNECT) && (0 !=  responseStatus); i++)
		{
			responseStatus = radio->sendPacketWithResponse((uint32_t *)packet, 15);
			radio->disableRadio();

			if(RADIO_OK == responseStatus)
			{
				if(PACKET_init == ((init_packet_t *)packet)->packetType)
				{
					connectStatus = CONNECTED;
				}
				else
				{
					connectStatus = WRONG_PACKET_TYPE;
				}
			}
		}

		if(CONNECTED == connectStatus)
		{
			channel = ((init_packet_t *)packet)->channel;
			connectedFlag = 1;

			PPI_Init();
			RTC0_Init(((init_packet_t *)packet)->rtc_val_CC0, ((init_packet_t *)packet)->rtc_val_CC1);

			if(waitForSync(200))
			{
				radio->setChannel(channel);
				RTC0->TASKS_CLEAR = 1U;
				RTC0->TASKS_START = 1U;
				radio->setPacketPtr((uint32_t)packet);
				radio->endInterruptEnable();
				radio->readyToStartShortcutSet();
				radio->endToDisableShortcutSet();
			}
			else
			{
				error();
			}
		}
		else
		{
			connectStatus = DISCONNECTED;
			stopHFCLK();
			sleep();
		}

		return connectStatus;
	}
	else
	{
		return ALREADY_CONNECTED;
	}
}

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
static void RTC0_Init(uint32_t valueOfCC0, uint32_t valueOfCC1)
{
	RTC0->CC[0] = valueOfCC0;
	RTC0->CC[1] = valueOfCC1;

	RTC0->PRESCALER = 0;
	RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos; 	// enable generate event for compare0
	RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
	RTC0->INTENSET = RTC_INTENSET_COMPARE1_Enabled << RTC_INTENSET_COMPARE1_Pos;

	NVIC_SetPriority(RTC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), HIGH_IRQ_PRIO, RTC0_INTERRUPT_PRIORITY));
	NVIC_EnableIRQ(RTC0_IRQn);
}

//=======================================================================================
static uint8_t waitForSync(uint16_t ms)
{
	radio->setChannel(SYNC_CHANNEL);

	if(RADIO_OK == radio->readPacketWithTimeout((uint32_t *)packet, ms))
		return (PACKET_sync == ((sync_packet_t *)packet)->packetType);
	else
		return FALSE;
}

//=======================================================================================
inline void radioSensorHandler()
{
	if(syncFlag)
	{
		syncFlag = 0;

		if(radio->checkCRC())
		{
			if(PACKET_sync == ((sync_packet_t *)packet)->packetType)
			{
				RTC0->TASKS_CLEAR = 1U;

				radio->setChannel(channel);

				radioRxStatus = RADIO_OK;
			}
			else
			{
				RTC0->TASKS_CLEAR = 1U;									// sync, but data was not properly readed
				radioRxStatus = RADIO_ACKError;							// TODO zmienic na wrong type of packet
			}
		}
		else
		{
			RTC0->TASKS_CLEAR = 1U;										// sync, but data was not properly readed
			radioRxStatus = RADIO_CRCError;
		}
	}
}

//=======================================================================================
inline void timeSlotHandler()
{
	((data_packet_t *)packet)->payloadSize = packetLength - 1;
	((data_packet_t *)packet)->packetType = PACKET_data;
	((data_packet_t *)packet)->channel = channel;
	
	timeSlotCallback();

	while ( !isHFCLKstable() )				// wait for stable HCLK which has been started via RTC0 event through PPI
		;

	radio->txEnable();

	gpioGeneratePulse(SCL_PIN);
	LED_1_TOGGLE();
}

//=======================================================================================
inline void syncHandler()
{
	radio->setChannel(SYNC_CHANNEL);
	syncFlag = 1;
	
	if ( 0 > startHFCLK() )
		error();
	
	radio->rxEnable();
	
	gpioGeneratePulse(SCL_PIN);
	LED_2_TOGGLE();
}

