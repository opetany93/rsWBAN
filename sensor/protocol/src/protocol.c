#include "protocol.h"
#include "radio.h"
#include "rtc.h"
#include "clocks.h"
#include "nrf_gpio.h"
#include "nrf_ppi.h"
#include "mydefinitions.h"
#include "hal.h"
#include <stddef.h>

#include "nrf_rtc.h"
#include "nrf_spim.h"

#define ATTEMPTS_OF_CONNECT		20

char packet[PACKET_SIZE];
static sync_packet_t* syncPacketPtr = (sync_packet_t* )packet;

volatile uint8_t channel, syncFlag, connectedFlag = 0;

static Radio* radio = NULL;
static Rtc* rtc = NULL;

static SensorCallbacks_t callbacks;

// --------------- internal functions -------------
static void initPPI(void);
static void configRtc(void);
static void setRtcValues(init_packet_t* initPacket);
static bool waitForSync(uint16_t ms);
static inline void prepareDataPacket(data_packet_t* dataPacket);
static inline protocol_status_t tryConnect(init_packet_t* initPacket);
// -------------------------------------------------

//=======================================================================================
void initProtocol(Radio* radioDrv, Rtc* rtcDrv, SensorCallbacks_t this)
{
	radio = radioDrv;
	rtc = rtcDrv;
	callbacks = this;

	configRtc();
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
	init_packet_t* initPacketPtr = (init_packet_t* )packet;
	
	if(!connectedFlag)
	{
		startHFCLK();
		radio->setChannel(BROADCAST_CHANNEL); 						// Frequency bin 30, 2430MHz
		connectStatus = tryConnect(initPacketPtr);

		if(CONNECTED == connectStatus)
		{
			channel = initPacketPtr->channel;
			connectedFlag = 1;

			initPPI();
			setRtcValues(initPacketPtr);

			if(waitForSync(200))
			{
				rtc->start(rtc);

				radio->setChannel(channel);
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
static inline protocol_status_t tryConnect(init_packet_t* initPacket)
{
	protocol_status_t connectStatus = DISCONNECTED;
	RADIO_status_t responseStatus = RADIO_NOT_OK;
	
	initPacket->payloadSize = 1;
	initPacket->packetType = PACKET_init;

	for(uint8_t i = 0; (i < ATTEMPTS_OF_CONNECT) && (RADIO_OK !=  responseStatus); i++)
	{
		responseStatus = radio->sendPacketWithResponse((uint32_t *)packet, 15);
		radio->disableRadio();

		if(RADIO_OK == responseStatus)
		{
			if(isPacketInit(initPacket))
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

static inline void setRtcValues(init_packet_t* initPacket)
{
	rtc->setCCreg(rtc, RTC_CHANNEL0, initPacket->rtcValCC0);
	rtc->setCCreg(rtc, RTC_CHANNEL1, initPacket->rtcValCC1);
}

//=======================================================================================
static void initPPI()
{
	// connect RTC0 COMPARE[0] EVENT to CLOCK HFCLKSTART TASK
	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL0, (uint32_t) &NRF_RTC0->EVENTS_COMPARE[0], (uint32_t) &NRF_CLOCK->TASKS_HFCLKSTART);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL0);

	// connect RADIO DISABLED EVENT to CLOCK TASK HFCLKSTOP
	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL1, (uint32_t) &NRF_RADIO->EVENTS_DISABLED, (uint32_t) &NRF_CLOCK->TASKS_HFCLKSTOP);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL1);
}

//=======================================================================================
static void configRtc()
{
	rtc->setPrescaler(rtc, 0);
	rtc->compareEventEnable(rtc, RTC_CHANNEL0);
	rtc->compareEventEnable(rtc, RTC_CHANNEL1);
	rtc->compareInterruptEnable(rtc, RTC_CHANNEL0);
	rtc->compareInterruptEnable(rtc, RTC_CHANNEL1);
	rtc->clear(rtc);
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
				if(checkApprovals(syncPacketPtr, channel))
				{
					SYSTEM_OFF();
				}

				rtc->clear(rtc);
				radio->setChannel(channel);

				if (NULL != callbacks.syncCallback)
					callbacks.syncCallback();
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
inline void timeSlotHandler()
{
	data_packet_t* dataPacket = (data_packet_t* )packet;

	prepareDataPacket(dataPacket);

	if(NULL != callbacks.timeSlotCallback)
		callbacks.timeSlotCallback(dataPacket);

	while (!isHFCLKstable())				// wait for stable HCLK which has been started via RTC0 event through PPI
		;
	
	radio->txEnable();
}

//=======================================================================================
static inline void prepareDataPacket(data_packet_t* dataPacket)
{
	dataPacket->payloadSize = sizeof(data_packet_t) - 1;
	dataPacket->packetType = PACKET_data;
	dataPacket->channel = channel;
	dataPacket->numberOfPacket++;

	if(!connectedFlag)
	{
		dataPacket->disconnect = 1;
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
	
	LED_2_TOGGLE();
}
//=======================================================================================
