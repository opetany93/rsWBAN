#include "protocol_host.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "radio.h"
#include "lcd_Nokia5110.h"
#include "mytypes.h"
#include "mydefinitions.h"
#include "hal.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_rtc.h"
#include "nrf_ppi.h"

char packet[PACKET_SIZE];
static data_packet_t* packetAsData = (data_packet_t* )packet;
static init_packet_t* packetAsInit = (init_packet_t* )packet;
static sync_packet_t* packetAsSync = (sync_packet_t* )packet;
volatile data_packet_t* packets[4];

volatile uint8_t flagsOfConnectedSensors = 0;
volatile uint8_t amountOfConnectedSensors = 0;

volatile uint8_t channel;

volatile uint32_t rtc_val_CC0_base;
volatile uint32_t rtc_val_CC1;

volatile uint8_t txPower = 1, turnOff = 0, approvals = 0;

static Radio* radio = NULL;
static Rtc* rtc0 = NULL;
static Rtc* rtc1 = NULL;

static void startListening(void);
static void setFreqCollectData(uint8_t freq);

static void sufInit(void);
static void timeSlotsInit(void);
static void initPPI(void);
static void prepareSyncPacket();
static void prepareInitPacket(uint8_t numberOfSlot);
static uint8_t findFreeTimeSlot();
static void addSensor(uint8_t numberOfSlot);
static void removeSensor(uint8_t numberOfSlot);
static void changeRadioSlotChannel(uint8_t channel);
static inline bool isPacketData(data_packet_t* packetPtr);
static inline bool isPacketInit(init_packet_t* packetPtr);

// =======================================================================================
__WEAK void dataReadyCallback(data_packet_t** packets, uint8_t amountOfConnectedSensors)
{

}

// =======================================================================================
Protocol* initProtocol(Radio* radioDrv, Rtc* rtc0Drv, Rtc* rtc1Drv)
{
	Protocol *protocol = malloc(sizeof(Protocol));
	protocol->setFreqCollectData = setFreqCollectData;
	protocol->startListening = startListening;

	radio = radioDrv;
	rtc0 = rtc0Drv;
	rtc1 = rtc1Drv;

	sufInit();
	timeSlotsInit();
	initPPI();

	return protocol;
}

// =======================================================================================
static void sufInit()
{
	rtc0->setCCreg(rtc0, RTC_CHANNEL0, 65);			// number of RTC0 impulses after which starts determining the time slots are by RTC1
	rtc0->setCCreg(rtc0, RTC_CHANNEL1, 1638);		// determination of the SUF
	rtc0->setPrescaler(rtc0, 0);
	rtc0->compareEventEnable(rtc0, RTC_CHANNEL0);
	rtc0->compareInterruptEnable(rtc0, RTC_CHANNEL0);
	rtc0->compareEventEnable(rtc0, RTC_CHANNEL1);
	rtc0->compareInterruptEnable(rtc0, RTC_CHANNEL1);

	NVIC_SetPriority(RTC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), HIGH_IRQ_PRIO, RTC0_IRQ_PRIORITY));
	NVIC_EnableIRQ(RTC0_IRQn);

	rtc0->start(rtc0);
}

// =======================================================================================
static inline void timeSlotsInit()
{
	rtc1->setPrescaler(rtc1, 0);
	rtc1->compareEventEnable(rtc1, RTC_CHANNEL0);
	rtc1->compareInterruptEnable(rtc1, RTC_CHANNEL0);

	NVIC_SetPriority(RTC1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), HIGH_IRQ_PRIO, RTC1_IRQ_PRIORITY));
	NVIC_EnableIRQ(RTC1_IRQn);

	rtc1->setCCreg(rtc1, RTC_CHANNEL0, 392);
	rtc1->start(rtc1);
}

// =======================================================================================
static inline void initPPI()
{
	// when SYNC slot is over then start RTC1 for time slots
	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL0, (uint32_t) &RTC0->EVENTS_COMPARE[0], (uint32_t) &RTC1->TASKS_START);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL0);

	// when SUF is over start again
	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL1, (uint32_t) &RTC0->EVENTS_COMPARE[1], (uint32_t) &RTC0->TASKS_CLEAR);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL1);

	// when SUF is over then STOP and CLEAR RTC1
	nrf_ppi_channel_and_fork_endpoint_setup(NRF_PPI_CHANNEL2, (uint32_t) &RTC0->EVENTS_COMPARE[1], (uint32_t) &RTC1->TASKS_STOP, (uint32_t) &RTC1->TASKS_CLEAR);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL2);

	// when given time slot is over then CLEAR RTC1 and start again with the next time slot
	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL3, (uint32_t) &RTC1->EVENTS_COMPARE[0], (uint32_t) &RTC1->TASKS_CLEAR);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL3);
}

// =======================================================================================
inline void startListening()
{
	radio->setPacketPtr((uint32_t)packet);
	radio->rxEnable();								// Enable radio and wait for ready
	rtc0->start(rtc0);
}

// =======================================================================================
inline void radioHostHandler()
{
	if(radio->isRxIdleState())
	{
		LED_1_TOGGLE();
		
		if(radio->checkCRC())
		{
			if(isPacketData(packetAsData))
			{
				memcpy((void*)packets[packetAsData->channel], packet, sizeof(data_packet_t));

				if(1 == packetAsData->disconnect)
				{
					removeSensor(packetAsData->channel);
					approvals |= (1 << packetAsData->channel);
				}
			}
			
			else if(isPacketInit(packetAsInit))
			{
				uint8_t freeSlot = findFreeTimeSlot();
				
				addSensor(freeSlot);
				prepareInitPacket(freeSlot);

				radio->sendPacket((uint32_t *)packet);
				radio->disableRadio();
			}
		}
	}
}

// =======================================================================================
static uint8_t findFreeTimeSlot()
{
	uint8_t freeSlot;

	for(freeSlot = 0; (flagsOfConnectedSensors & (1 << freeSlot)); freeSlot++)		// searching for a free time slot
		;

	return freeSlot;
}

// =======================================================================================
static void addSensor(uint8_t numberOfSlot)
{
	flagsOfConnectedSensors |= (1 << numberOfSlot);									// sensor is connected
	amountOfConnectedSensors++;

	packets[numberOfSlot] = (data_packet_t *)malloc(sizeof(data_packet_t));
}

// =======================================================================================
static void removeSensor(uint8_t numberOfSlot)
{
	flagsOfConnectedSensors &= ~(1 << numberOfSlot);
	amountOfConnectedSensors--;

	free((data_packet_t*)packets[numberOfSlot]);
}

// =======================================================================================
static void prepareInitPacket(uint8_t numberOfSlot)
{
	packetAsInit->channel = numberOfSlot;							// przypisanie id sensora
	packetAsInit->payloadSize = sizeof(init_packet_t);
	packetAsInit->packetType = PACKET_init;
	packetAsInit->rtcValCC0 = rtc_val_CC0_base * ( (2 * (numberOfSlot + 1)) - 1);
	packetAsInit->rtcValCC1 = rtc_val_CC1;
}

// =======================================================================================
inline void syncTransmitHandler()
{
	radio->disableRadio();
	radio->setChannel(SYNC_CHANNEL);
	radio->endInterruptDisable();
	
	prepareSyncPacket();

	//nrf_gpio_pin_toggle(ARDUINO_0_PIN);
	gpioGeneratePulse(ARDUINO_0_PIN);

	radio->sendPacket((uint32_t *)packet);							// send sync packet
	radio->disableRadio();
	radio->clearFlags();
	radio->endInterruptEnable();

	approvals = 0;

	dataReadyCallback((data_packet_t**)packets, amountOfConnectedSensors);
}

// =======================================================================================
static inline void prepareSyncPacket()
{
	packetAsSync->payloadSize = sizeof(sync_packet_t) - 1;
	packetAsSync->packetType = PACKET_sync;
	packetAsSync->rtcValCC0 = 0;
	packetAsSync->rtcValCC1 = 0;

	packetAsSync->approvals = approvals;
	packetAsSync->txPower = txPower;
	packetAsSync->turnOff = turnOff;
}

// =======================================================================================
void startTimeSlotListener()
{
	radio->rxEnable();									// Enable radio and wait for ready

	channel = FIRST_CHANNEL;

	timeSlotListenerHandler();
}

// =======================================================================================
inline void timeSlotListenerHandler()
{
	if ( flagsOfConnectedSensors & (1 << channel) )
	{
		changeRadioSlotChannel(channel);
		nrf_gpio_pin_toggle(ARDUINO_1_PIN);

	}
	else if( channel != BROADCAST_CHANNEL )
	{
		changeRadioSlotChannel(BROADCAST_CHANNEL);
		nrf_gpio_pin_toggle(ARDUINO_2_PIN);
	}

	channel++;
}

// =======================================================================================
static void changeRadioSlotChannel(uint8_t channel)
{
	radio->disableRadio();
	radio->setChannel(channel);
	radio->clearFlags();
	radio->rxEnable();								// Enable radio and wait for ready
}

static inline bool isPacketData(data_packet_t* packetPtr)
{
	return PACKET_data == packetPtr->packetType;
}

static inline bool isPacketInit(init_packet_t* packetPtr)
{
	return PACKET_init == packetPtr->packetType;
}

// =======================================================================================
void lcdProtocolInit()
{
	// LCD init
	nrf_gpio_cfg_output(2);
	nrf_gpio_pin_clear(2);
	
	nrf_gpio_cfg_output(25);
	nrf_gpio_pin_set(25);
	
	lcd_init();
	lcd_draw_text(0, 32, "HOST");
	lcd_draw_line(0, 8, 83, 8);
	lcd_draw_text(3, 0, "Sensors: ");
	lcd_copy();
	
	//24bit mode
	TIMER1->BITMODE = TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos;
	//Enable interrupt for COMPARE[0]
	TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Enabled <<  TIMER_INTENSET_COMPARE0_Pos;
	
	NVIC_SetPriority(TIMER1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), LOW_IRQ_PRIO, 2));
	NVIC_EnableIRQ(TIMER1_IRQn);
	
	TIMER1->CC[0] = 100 * 1000;		// 100 ms
	TIMER1->TASKS_START = 1U;
}

// =======================================================================================
void setFreqCollectData(uint8_t freq)
{
	switch (freq)
	{
		case FREQ_COLLECT_DATA_100mHz:
			rtc_val_CC0_base = 40957;
			rtc_val_CC1 = 327621;
			RTC0->CC[0] = 327654;
			RTC1->CC[0] = 81913;
		
			lcd_draw_text(2, 0, "Freq: 0,1 Hz");
			break;

		case FREQ_COLLECT_DATA_1Hz:
			rtc_val_CC0_base = 4096;
			rtc_val_CC1 = 32733;
			RTC0->CC[0] = 32767;
			RTC1->CC[0] = 8191;
		
			lcd_draw_text(2, 0, "Freq: 1 Hz");
			break;
		
		case FREQ_COLLECT_DATA_10Hz:
			rtc_val_CC0_base = 410;
			rtc_val_CC1 = 3244;
			RTC0->CC[0] = 3277;
			RTC1->CC[0] = 819;
		
			lcd_draw_text(2, 0, "Freq: 10 Hz");
			break;
		
		case FREQ_COLLECT_DATA_20Hz:
			rtc_val_CC0_base = 205;
//			rtc_val_CC1 = 1606;
//			rtc_val_CC1 = 1596;		// 10 less beacuse is compensated time of sending and receiving of sync packet
			rtc_val_CC1 = 1638;
//			RTC0->CC[0] = 1638;
//			RTC1->CC[0] = 410;
		
			lcd_draw_text(2, 0, "Freq: 20 Hz");
			break;
	}
	
	lcd_copy();
}

