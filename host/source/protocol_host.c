#include "protocol_host.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "radio.h"
#include "lcd_Nokia5110.h"
#include "uart.h"
#include "mytypes.h"
#include "mydefinitions.h"
#include "hal.h"
#include "ADXL362.h"

#include "nrf_gpio.h"

char packet[PACKET_SIZE];

volatile data_packet_t *packet_ptr = (data_packet_t *)packet;
volatile data_packet_t *packets[4];

volatile uint8_t flagsOfConnectedSensors = 0;
volatile uint8_t amountOfConnectedSensors = 0;

volatile uint8_t channel;

volatile uint32_t rtc_val_CC0_base;
volatile uint32_t rtc_val_CC1;

extern char uartBuf[100];
volatile uint8_t packetLength = 30;
volatile uint8_t txPower = 1, turnOff = 0;

ADXL362_AXES_t axis;

Radio *radio = NULL;

void startListening(void);
void setFreqCollectData(uint8_t freq);

static void RTC0_SUF_init(void);
static void RTC1_timeSlotsInit(void);
static void initPPI(void);
static void prepareSyncPacket();
static void prepareInitPacket(uint8_t numberOfSlot);
static uint8_t findFreeTimeSlot();
static void addSensor(uint8_t numberOfSlot);
static void changeRadioSlotChannel(uint8_t channel);

Protocol* initProtocol(Radio* radioDrv)
{
	Protocol *protocol = malloc(sizeof(Protocol));
	protocol->setFreqCollectData = setFreqCollectData;
	protocol->startListening = startListening;

	radio = radioDrv;

	RTC0_SUF_init();
	RTC1_timeSlotsInit();
	initPPI();

	return protocol;
}

// =======================================================================================
static void RTC0_SUF_init()
{
	RTC0->CC[0] = 65;			// number of RTC0 impulses after which starts determining the time slots are by RTC1
	RTC0->CC[1] = 1638;			// determination of the SUF

	RTC0->PRESCALER = 0;

	RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
	RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
	RTC0->EVTENSET = RTC_EVTENSET_COMPARE1_Enabled << RTC_EVTENSET_COMPARE1_Pos;
	RTC0->INTENSET = RTC_INTENSET_COMPARE1_Enabled << RTC_INTENSET_COMPARE1_Pos;

	NVIC_SetPriority(RTC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), HIGH_IRQ_PRIO, RTC0_IRQ_PRIORITY));
	NVIC_EnableIRQ(RTC0_IRQn);

	RTC0->TASKS_CLEAR = 1U;
}

// =======================================================================================
static void RTC1_timeSlotsInit()
{
	RTC1->PRESCALER = 0;

	RTC1->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
	RTC1->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;

	NVIC_SetPriority(RTC1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), HIGH_IRQ_PRIO, RTC1_IRQ_PRIORITY));
	NVIC_EnableIRQ(RTC1_IRQn);

	RTC1->CC[0] = 392;
	RTC1->TASKS_CLEAR = 1U;
}

// =======================================================================================
static inline void initPPI()
{
	// when SYNC slot is over then start RTC1 for time slots
	PPI->CH[0].EEP = (uint32_t) &RTC0->EVENTS_COMPARE[0];
	PPI->CH[0].TEP = (uint32_t) &RTC1->TASKS_START;
	PPI->CHENSET = PPI_CHENSET_CH0_Enabled << PPI_CHENSET_CH0_Pos;

	// when SUF is over start again
	PPI->CH[1].EEP = (uint32_t) &RTC0->EVENTS_COMPARE[1];
	PPI->CH[1].TEP = (uint32_t) &RTC0->TASKS_CLEAR;
	PPI->CHENSET = PPI_CHENSET_CH1_Enabled << PPI_CHENSET_CH1_Pos;

	// when SUF is over then STOP and CLEAR RTC1
	PPI->CH[2].EEP = (uint32_t) &RTC0->EVENTS_COMPARE[1];
	PPI->CH[2].TEP = (uint32_t) &RTC1->TASKS_STOP;
	PPI->FORK[2].TEP = (uint32_t) &RTC1->TASKS_CLEAR;
	PPI->CHENSET = PPI_CHENSET_CH2_Enabled << PPI_CHENSET_CH2_Pos;

	// when given time slot is over then CLEAR RTC1 and start again with the next time slot
	PPI->CH[3].EEP = (uint32_t) &RTC1->EVENTS_COMPARE[0];
	PPI->CH[3].TEP = (uint32_t) &RTC1->TASKS_CLEAR;
	PPI->CHENSET = PPI_CHENSET_CH3_Enabled << PPI_CHENSET_CH3_Pos;
}

// =======================================================================================
inline void startListening()
{
	RADIO->PACKETPTR = (uint32_t)&packet;
	RADIO->TASKS_RXEN = 1U;								// Enable radio and wait for ready
	
	RTC0->TASKS_START = 1U;
}

// =======================================================================================
inline void radioHostHandler()
{
	if(radio->isRxIdleState())
	{
		LED_1_TOGGLE();
		
		if(radio->checkCRC())
		{
			if(PACKET_data == packet_ptr->type)
			{
				memcpy((void*)packets[packet_ptr->channel], packet, sizeof(data_packet_t));
			}
			
			else if(PACKET_init == packet_ptr->type)
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

	for(freeSlot = 0; (flagsOfConnectedSensors & (1 << freeSlot)); freeSlot++);		// searching for a free time slot

	return freeSlot;
}

// =======================================================================================
static void addSensor(uint8_t numberOfSlot)
{
	flagsOfConnectedSensors |= (1 << numberOfSlot);								// sensor is connected
	amountOfConnectedSensors++;

	packets[numberOfSlot] = (data_packet_t *)malloc(sizeof(data_packet_t));
}

// =======================================================================================
static void prepareInitPacket(uint8_t numberOfSlot)
{
	((init_packet_t *)packet)->channel = numberOfSlot;							// przypisanie id sensora
	((init_packet_t *)packet)->payloadSize = 8;
	((init_packet_t *)packet)->ack = ACK;
	((init_packet_t *)packet)->rtc_val_CC0 = rtc_val_CC0_base * ( (2 * (numberOfSlot + 1)) - 1);
	((init_packet_t *)packet)->rtc_val_CC1 = rtc_val_CC1;
}

// =======================================================================================
inline void syncTransmitHandler()
{
	radio->disableRadio();
	RADIO->FREQUENCY = SYNC_CHANNEL;
	RADIO_END_INT_DISABLE();
	
	prepareSyncPacket();

	NRF_GPIO->OUTSET = (1 << ARDUINO_0_PIN);
	NRF_GPIO->OUTCLR = (1 << ARDUINO_0_PIN);

	radio->sendPacket((uint32_t *)packet);							// send sync packet
	radio->disableRadio();
	RADIO->EVENTS_READY = 0U;
	RADIO->EVENTS_END = 0U;
	RADIO_END_INT_ENABLE();

	sendDataToPC();
}

// =======================================================================================
static inline void prepareSyncPacket()
{
	((sync_packet_t *)packet)->payloadSize = sizeof(sync_packet_t) - 1;
	((sync_packet_t *)packet)->sync = SYNC;
	((sync_packet_t *)packet)->rtc_val_CC0 = 0;
	((sync_packet_t *)packet)->rtc_val_CC1 = 0;

	((sync_packet_t *)packet)->txPower = txPower;
	((sync_packet_t *)packet)->turnOff = turnOff;
}

// =======================================================================================
void sendDataToPC()
{
	if( amountOfConnectedSensors == 1 )
	{
		sprintf(uartBuf, "%d %d %d\r\n", packets[0]->axes.x,  packets[0]->axes.y, packets[0]->axes.z);
		uartWriteS(uartBuf);
	}

//	if( connected_sensors_amount == 1 )
//	{
//		for(uint8_t i = 0; i < 15; i++)
//		{
//			axisTemp = packets[0]->data[i];
//
//			if ( (axisTemp >> 14) == 1 )
//			{
//				axisTemp <<= 2;
//				axisTemp >>= 2;
//
//				axisTemp |= (axisTemp >> 12) << 14;
//
//				axis.y = axisTemp;
//			}
//			else if ( (axisTemp >> 14) == 2 )
//			{
//				axisTemp <<= 2;
//				axisTemp >>= 2;
//
//				axisTemp |= (axisTemp >> 12) << 14;
//
//				axis.z = axisTemp;
//			}
//			else if ( (axisTemp >> 14) == 0 )
//			{
//				axisTemp <<= 2;
//				axisTemp >>= 2;
//
//				axisTemp |= (axisTemp >> 12) << 14;
//
//				axis.x = axisTemp;
//			}
//
//			if ( !((i+1) % 3) )
//			{
//				//uartDmaWriteS(buf, sprintf(buf, "%d %d %d\r\n", axis.x, axis.y, axis.z));
//				sprintf(buf, "%d %d %d\r\n", axis.x, axis.y, axis.z);
//				uartWriteS(buf);
//			}
//		}
//	}

	if( amountOfConnectedSensors == 2 )
	{
		sprintf(uartBuf, "%d %d %d %d\r\n", packets[0]->axes.x,  packets[0]->axes.y, packets[0]->axes.z, packets[1]->axes.x);
		uartWriteS(uartBuf);
	}

	if( amountOfConnectedSensors == 3 )
	{
		sprintf(uartBuf, "%d %d %d %d %d\r\n", packets[0]->axes.x,  packets[0]->axes.y, packets[0]->axes.z, packets[1]->axes.x, packets[2]->axes.x);
		uartWriteS(uartBuf);
	}

	if( amountOfConnectedSensors == 4 )
	{
		sprintf(uartBuf, "%d %d %d %d %d %d\r\n", packets[0]->axes.x,  packets[0]->axes.y, packets[0]->axes.z, packets[1]->axes.x, packets[2]->axes.x, packets[3]->axes.x);
		uartWriteS(uartBuf);
	}
}

// =======================================================================================
void startTimeSlotListener()
{
	RADIO->TASKS_RXEN = 1U;									// Enable radio and wait for ready

	channel = FIRST_CHANNEL;

	timeSlotListenerHandler();
}

// =======================================================================================
void timeSlotListenerHandler()
{
	if ( flagsOfConnectedSensors & (1 << channel) )
	{
		changeRadioSlotChannel(channel);
	}
	else if( channel != ADVERTISEMENT_CHANNEL )
	{
		changeRadioSlotChannel(ADVERTISEMENT_CHANNEL);
	}

	channel++;
}

// =======================================================================================
static void changeRadioSlotChannel(uint8_t channel)
{
	radio->disableRadio();

	RADIO->FREQUENCY = channel;
	RADIO->EVENTS_READY = 0U;
	RADIO->EVENTS_END  = 0U;
	RADIO->TASKS_RXEN = 1U;								// Enable radio and wait for ready

	NRF_GPIO->OUTSET = (1 << ARDUINO_1_PIN);
	NRF_GPIO->OUTCLR = (1 << ARDUINO_1_PIN);
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
			rtc_val_CC1 = 1606;
//			RTC0->CC[0] = 1638;
//			RTC1->CC[0] = 410;
		
			lcd_draw_text(2, 0, "Freq: 20 Hz");
			break;
	}
	
	lcd_copy();
}

