#include "protocol.h"
#include "radio.h"
#include "clocks.h"
#include "nrf_gpio.h"
#include "mydefinitions.h"
#include "mytypes.h"
#include "hal.h"
#include <stddef.h>
#include <stdbool.h>

#define ATTEMPTS_OF_CONNECT		20

char packet[PACKET_SIZE];
data_packet_t* dataPacketPtr = (data_packet_t* )packet;
init_packet_t* initPacketPtr = (init_packet_t* )packet;
sync_packet_t* syncPacketPtr = (sync_packet_t* )packet;

volatile uint8_t channel, packetLength = 24, syncFlag, connectedFlag = 0;

Radio *radio = NULL;

// --------------- internal functions -------------
static void PPI_Init();
static void RTC0_Init(uint32_t valueOfCC0, uint32_t valueOfCC1);
static bool waitForSync(uint16_t ms);
static inline bool isPacketInit(init_packet_t* packetPtr);
static inline bool isPacketSync(sync_packet_t* packetPtr);
static inline bool checkApprovals(sync_packet_t* packetPtr);
static inline void prepareDataPacket();
// -------------------------------------------------

__WEAK void timeSlotCallback(data_packet_t* dataPacketPtr)
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
	LED_2_OFF();
	radio->disableRadio();
	connectedFlag = 0;
}

// =======================================================================================
protocol_status_t connect(void)
{
	protocol_status_t connectStatus = DISCONNECTED;
	RADIO_status_t responseStatus = 4;

	if(!connectedFlag)
	{
		startHFCLK();
		dataPacketPtr->payloadSize = 1;
		dataPacketPtr->packetType = PACKET_init;

		radio->setChannel(ADVERTISEMENT_CHANNEL); 						// Frequency bin 30, 2430MHz

		for(uint8_t i = 0; (i < ATTEMPTS_OF_CONNECT) && (RADIO_OK !=  responseStatus); i++)
		{
			responseStatus = radio->sendPacketWithResponse((uint32_t *)packet, 15);
			radio->disableRadio();

			if(RADIO_OK == responseStatus)
			{
				if(isPacketInit(initPacketPtr))
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
			channel = initPacketPtr->channel;
			connectedFlag = 1;

			PPI_Init();
			RTC0_Init(initPacketPtr->rtc_val_CC0, initPacketPtr->rtc_val_CC1);

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
static bool waitForSync(uint16_t ms)
{
	radio->setChannel(SYNC_CHANNEL);

	if(RADIO_OK == radio->readPacketWithTimeout((uint32_t *)packet, ms))
		return isPacketSync(syncPacketPtr);
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
			if(isPacketSync(syncPacketPtr))
			{
				if(checkApprovals(syncPacketPtr))
				{
					SYSTEM_OFF();
				}

				RTC0->TASKS_CLEAR = 1U;
				radio->setChannel(channel);
			}
			else
			{
				RTC0->TASKS_CLEAR = 1U;							// sync, but data was not properly readed
			}
		}
		else
		{
			RTC0->TASKS_CLEAR = 1U;								// sync, but data was not properly readed
		}
	}
}

//=======================================================================================
inline void timeSlotHandler()
{
	prepareDataPacket();
	timeSlotCallback(dataPacketPtr);

	while ( !isHFCLKstable() )				// wait for stable HCLK which has been started via RTC0 event through PPI
		;
	radio->txEnable();

	gpioGeneratePulse(SCL_PIN);
}

static inline void prepareDataPacket()
{
	dataPacketPtr->payloadSize = sizeof(data_packet_t) - 1;
	dataPacketPtr->packetType = PACKET_data;
	dataPacketPtr->channel = channel;
	dataPacketPtr->numberOfPacket++;

	if(!connectedFlag)
	{
		dataPacketPtr->disconnect = 1;
	}
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

static inline bool isPacketInit(init_packet_t* packetPtr)
{
	return PACKET_init == packetPtr->packetType;
}

static inline bool isPacketSync(sync_packet_t* packetPtr)
{
	return PACKET_sync == packetPtr->packetType;
}

static inline bool checkApprovals(sync_packet_t* packetPtr)
{
	return packetPtr->approvals & (1 << channel);
}
