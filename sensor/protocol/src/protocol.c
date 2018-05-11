#include "protocol.h"
#include "radio.h"
#include "rtc.h"
#include "clocks.h"
#include "nrf_gpio.h"
#include "nrf_ppi.h"
#include "mydefinitions.h"
#include "hal.h"
#include <stddef.h>
#include <stdbool.h>

#define ATTEMPTS_OF_CONNECT		20

char packet[PACKET_SIZE];
static data_packet_t* dataPacketPtr = (data_packet_t* )packet;
static init_packet_t* initPacketPtr = (init_packet_t* )packet;
static sync_packet_t* syncPacketPtr = (sync_packet_t* )packet;

volatile uint8_t channel, syncFlag, connectedFlag = 0;

Radio* radio = NULL;
Rtc* rtc = NULL;

// --------------- internal functions -------------
static void PPI_Init();
static void setRtc(uint32_t valueOfCC0, uint32_t valueOfCC1);
static bool waitForSync(uint16_t ms);
static inline bool isPacketInit(init_packet_t* packetPtr);
static inline bool isPacketSync(sync_packet_t* packetPtr);
static inline bool checkApprovals(sync_packet_t* packetPtr);
static inline void prepareDataPacket();
static inline protocol_status_t tryConnect();
// -------------------------------------------------

__WEAK void timeSlotCallback(data_packet_t* dataPacketPtr)
{

}

//=======================================================================================
void initProtocol(Radio* radioDrv, Rtc* rtcDrv)
{
	radio = radioDrv;
	rtc = rtcDrv;
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
	
	if(!connectedFlag)
	{
		startHFCLK();
		dataPacketPtr->payloadSize = 1;
		dataPacketPtr->packetType = PACKET_init;

		radio->setChannel(ADVERTISEMENT_CHANNEL); 						// Frequency bin 30, 2430MHz

		connectStatus = tryConnect();

		if(CONNECTED == connectStatus)
		{
			channel = initPacketPtr->channel;
			connectedFlag = 1;

			PPI_Init();
			setRtc(initPacketPtr->rtc_val_CC0, initPacketPtr->rtc_val_CC1);

			if(waitForSync(200))
			{
				radio->setChannel(channel);
				rtc->clear(rtc);
				rtc->start(rtc);
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
static inline protocol_status_t tryConnect()
{
	protocol_status_t connectStatus = DISCONNECTED;
	RADIO_status_t responseStatus = 4;
	
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
	
	return connectStatus;
}

//=======================================================================================
static void PPI_Init()
{
	// connect RTC0 COMPARE[0] EVENT to CLOCK HFCLKSTART TASK
	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL0, (uint32_t) &NRF_RTC0->EVENTS_COMPARE[0], (uint32_t) &NRF_CLOCK->TASKS_HFCLKSTART);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL0);

	// connect RADIO DISABLED EVENT to CLOCK TASK HFCLKSTOP
	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL1, (uint32_t) &NRF_RADIO->EVENTS_DISABLED, (uint32_t) &NRF_CLOCK->TASKS_HFCLKSTOP);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL1);
}

//=======================================================================================
static void setRtc(uint32_t valueOfCC0, uint32_t valueOfCC1)
{
	rtc->setCCreg(rtc, 0, valueOfCC0);
	rtc->setCCreg(rtc, 1, valueOfCC1);
	rtc->setPrescaler(rtc, 0);
	rtc->compareEventEnable(rtc, 0);
	rtc->compareInterruptEnable(rtc, 0);
	rtc->compareInterruptEnable(rtc, 1);
}

//=======================================================================================
static bool waitForSync(uint16_t ms)
{
	radio->setChannel(SYNC_CHANNEL);

	if(RADIO_OK == radio->readPacketWithTimeout((uint32_t *)packet, ms))
		return isPacketSync(syncPacketPtr);
	else
		return false;
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

				rtc->clear(rtc);
				radio->setChannel(channel);
			}
			else
			{
				rtc->clear(rtc);							// sync, but data was not properly readed
			}
		}
		else
		{
			rtc->clear(rtc); 								// sync, but data was not properly readed
		}
	}
}

//=======================================================================================
static inline bool isPacketSync(sync_packet_t* packetPtr)
{
	return PACKET_sync == packetPtr->packetType;
}

//=======================================================================================
static inline bool checkApprovals(sync_packet_t* packetPtr)
{
	return packetPtr->approvals & (1 << channel);
}

//=======================================================================================
static inline bool isPacketInit(init_packet_t* packetPtr)
{
	return PACKET_init == packetPtr->packetType;
}

//=======================================================================================
inline void timeSlotHandler()
{
	prepareDataPacket();
	timeSlotCallback(dataPacketPtr);

	while (!isHFCLKstable())				// wait for stable HCLK which has been started via RTC0 event through PPI
		;
	
	radio->txEnable();

	gpioGeneratePulse(SCL_PIN);
}

//=======================================================================================
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
//=======================================================================================
