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

#include "nrf_delay.h"
#include "nrf_gpio.h"

volatile uint16_t 				lost_packet_amount[4]={0, 0, 0, 0};
volatile uint16_t 				good_packet_amount[4]={0, 0, 0, 0};

char 							packet[PACKET_SIZE];

volatile data_packet_t 			*packet_ptr = (data_packet_t *)packet;
volatile data_packet_t 			*packets[4];

volatile uint8_t 				connected_sensors_flags = 0;
volatile uint8_t 				connected_sensors_amount = 0;

volatile uint8_t 				channel = FIRST_CHANNEL;

volatile int 					rtc_val_CC0_base;
volatile int 					rtc_val_CC1;

extern char buf[100];

volatile uint8_t packetLength = 30;

volatile double resultOfCalculatingPER;
volatile double resultsOfTests[16];
volatile uint8_t cnt = 0, testDone = 0, txPower = 1, turnOff = 0;

static void testProtocolWithRadioPower(void);
static double calculatePER(int numberOfPacketsToCalculatePER);

uint8_t doneTest = FALSE;

ADXL362_AXES_t axis;

uint16_t axisTemp;

// =======================================================================================
void RTC0_Sync()
{
	RTC0->PRESCALER = 0;
	
	// enable generate event for compare0
	RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
	RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
	
	// =============== for measurement ticks from RTC ===================================
	PPI->CH[2].EEP = (uint32_t) &RTC0->EVENTS_TICK;
	PPI->CH[2].TEP = (uint32_t) &GPIOTE->TASKS_OUT[0];
	PPI->CHENSET = PPI_CHENSET_CH2_Enabled << PPI_CHENSET_CH2_Pos;

	RTC0->EVTENSET = RTC_EVTENSET_TICK_Enabled << RTC_EVTENSET_TICK_Pos;

	GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos)
					  | (ARDUINO_0_PIN << GPIOTE_CONFIG_PSEL_Pos)
					  | (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos);

	// ===================================================================================

	NVIC_SetPriority(RTC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 
									 HIGH_IRQ_PRIO, RTC0_IRQ_PRIORITY));
	NVIC_EnableIRQ(RTC0_IRQn);
}

// =======================================================================================
void RTC1_TimeSlot()
{
	RTC1->PRESCALER = 0;
	
	// enable generate event for compare0
	RTC1->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
	RTC1->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
	
	NVIC_SetPriority(RTC1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 
									 HIGH_IRQ_PRIO, RTC1_IRQ_PRIORITY));
	
	NVIC_EnableIRQ(RTC1_IRQn);
}

// =======================================================================================
void startListening()
{
	RADIO->PACKETPTR = (uint32_t)&packet;
	RADIO->TASKS_RXEN = 1U;								// Enable radio and wait for ready
	
	RTC0->TASKS_START = 1U;								// RTCs starts
	RTC1->TASKS_START = 1U;
}

void radioHostHandler()
{
	volatile uint8_t free_slot;
	
	if( RADIO->STATE == RADIO_STATE_STATE_RxIdle )
	{
		LED_1_TOGGLE();
		
		if(RADIO->CRCSTATUS == 1U)
		{
			if( PACKET_data == packet_ptr->type )	
			{
				memcpy((void*)packets[packet_ptr->channel], packet, sizeof(data_packet_t));
			}
			
			else if( PACKET_init == packet_ptr->type )
			{
				for(free_slot = 0; (connected_sensors_flags & (1 << free_slot)); free_slot++);		// szukanie wolnej szczeliny
				((init_packet_t *)packet)->channel = free_slot;																									// przypisanie id sensora
				
				connected_sensors_flags |= (1 << free_slot);																			// sensor is connected
				
				packets[free_slot] = (data_packet_t *)malloc(sizeof(data_packet_t));	
				
				((init_packet_t *)packet)->payloadSize = 8;
				((init_packet_t *)packet)->ack = ACK;
				
				connected_sensors_amount++;
				((init_packet_t *)packet)->rtc_val_CC0 = rtc_val_CC0_base * ( (2 * (free_slot + 1)) - 1);
				((init_packet_t *)packet)->rtc_val_CC1 = rtc_val_CC1;
				
				packet_ptr->type = 0;
				
				sendPacket((uint32_t *)packet);
				disableRadio();
			}

			if( !doneTest )
			{
				good_packet_amount[0]++;
			}
		}
		else
		{
			if( !doneTest )
			{
				lost_packet_amount[0]++;
			}

//			for(uint8_t i = 0; i < 15; i++)
//			{
//				packets[0]->data[i] |= 2000;
//			}
		}
	}
}

void syncTransmitHandler()
{
	sendDataToPC();

	nrf_gpio_pin_set(ARDUINO_1_PIN);

	disableRadio();
		
	RADIO->FREQUENCY = SYNC_CHANNEL;
	
	RADIO_END_INT_DISABLE();
	
	((sync_packet_t *)packet)->payloadSize = 8;
	((sync_packet_t *)packet)->sync = SYNC;
	((sync_packet_t *)packet)->rtc_val_CC0 = 0;
	((sync_packet_t *)packet)->rtc_val_CC1 = 0;

	((sync_packet_t *)packet)->testPacketLength = packetLength;
	((sync_packet_t *)packet)->txPower = txPower;
	((sync_packet_t *)packet)->turnOff = turnOff;

	sendPacket((uint32_t *)packet);							// send sync packet
	disableRadio();
	RADIO->EVENTS_READY = 0U;
	RADIO->EVENTS_END = 0U;
	RADIO_END_INT_ENABLE();
	
	nrf_gpio_pin_clear(ARDUINO_1_PIN);

	// set ptr to packet and start RX
	RADIO->PACKETPTR = (uint32_t)packet;
	RADIO->TASKS_RXEN = 1U;									// Enable radio and wait for ready
	
	channel = FIRST_CHANNEL;
	RTC0->TASKS_CLEAR = 1U;

	NVIC_SetPendingIRQ(RTC1_IRQn);
}

void timeSlotListenerHandler()
{
	RTC1->TASKS_CLEAR = 1U;

	nrf_gpio_pin_toggle(31);								// toggle pin for debug
	
	if ( connected_sensors_flags & (1 << channel) )
	{
		disableRadio();
		
		RADIO->FREQUENCY = channel;
		RADIO->EVENTS_READY = 0U;
		RADIO->EVENTS_END  = 0U;
		RADIO->TASKS_RXEN = 1U;								// Enable radio and wait for ready

		//nrf_gpio_pin_toggle(12);							// toggle pin for debug
	}
	else if( channel != ADVERTISEMENT_CHANNEL )
	{
		disableRadio();
		
		RADIO->FREQUENCY = ADVERTISEMENT_CHANNEL;
		RADIO->EVENTS_READY = 0U;
		RADIO->EVENTS_END  = 0U;
		RADIO->TASKS_RXEN = 1U;								// Enable radio and wait for ready
	}
	channel++;
}

void sendDataToPC()
{
//	if(!doneTest)
//	{
//		testProtocolWithRadioPower();
//	}

//	if(!donePER)
//	{
//		calculatePER();
//	}

	if( connected_sensors_amount == 1 )
	{
		sprintf(buf, "%d %d %d\r\n", packets[0]->axes.x,  packets[0]->axes.y, packets[0]->axes.z);
		uartWriteS(buf);
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

//	sprintf(buf, "%d, %d, %d\r\n", packets[0]->data);
//	uartWriteS(buf);


//	if( connected_sensors_amount == 2 )
//	{
//		sprintf(buf, "%d, %d, %d, %d, %d, %d, %d, %d\r\n", packets[0]->axes.x, packets[0]->data[4],
//																			   packets[0]->data[140], packets[0]->data[50],
//																			   packets[1]->axes.x, packets[1]->data[4],
//																			   packets[1]->data[140], packets[1]->data[50] );
//		uartWriteS(buf);
//	}

//		if( connected_sensors_amount == 3 ) 
//		{
//			sprintf(buf, "%d, %d, %d, %d, %d\r\n", packets[0]->axes.x, packets[0]->axes.y, packets[0]->axes.z, packets[1]->axes.x, packets[2]->axes.x);
//			uartWriteS(buf);
//		}
//		
//		if( connected_sensors_amount == 4 ) 
//		{
//			sprintf(buf, "%d, %d, %d, %d, %d, %d\r\n", packets[0]->axes.x, packets[0]->axes.y, packets[0]->axes.z, packets[1]->axes.x, packets[2]->axes.x, packets[3]->axes.x);
//			uartWriteS(buf);
//		}
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
			RTC0->CC[0] = 1638;
			RTC1->CC[0] = 410;
		
			lcd_draw_text(2, 0, "Freq: 20 Hz");
			break;
	}
	
	lcd_copy();
}


// =================================================================================================================
static double calculatePER(int numberOfPacketsToCalculatePER)
{
	uint16_t sumOfReceivedPackets = good_packet_amount[0] + lost_packet_amount[0];

	if( sumOfReceivedPackets == numberOfPacketsToCalculatePER )
	{
		double per = (double)((double)lost_packet_amount[0] / (double)(sumOfReceivedPackets));

		sprintf(buf, "Wyslanych wszystkich pakietow: %d\r\n"
				     "Zle odebrane pakiety: %d \r\n"
				     "Dobrze odebrane pakiety: %d \r\n"
				     "Wspolczynnik PER: %f\n\r\n\r",
				   sumOfReceivedPackets, lost_packet_amount[0], good_packet_amount[0], per);

		uartWriteS(buf);

		//donePER = TRUE;
		lost_packet_amount[0] = 0;
		good_packet_amount[0] = 0;

		return per;
	}
//	else if( (!(sumOfReceivedPackets % 300)) && (sumOfReceivedPackets != 0) )
//	{
//		sprintf(buf,"Liczenie wspolczynnika PER w trakcie, postep: %d procent \n\r"
//				"Zle odebrane pakiety: %d\n\r", (int)(((double)sumOfReceivedPackets/(double)36000) * 100), lost_packet_amount[0] );
//
//		uartWriteS(buf);
//
//		return -1;
//	}

	return -1;
}

static void testProtocolWithRadioPower()
{
	resultOfCalculatingPER = calculatePER(3000);

	if ( (0 <= resultOfCalculatingPER) && (turnOff == 0) )
	{
		resultsOfTests[cnt++] = resultOfCalculatingPER;

		if ( 8 > txPower )
		{
			switch(++txPower)
			{
				case 1:
					sprintf(buf, "\nPower of radio in sensor was changed to -20 dBm.\n\r");
					break;

				case 2:
					sprintf(buf, "\nPower of radio in sensor was changed to -16 dBm.\n\r");
					break;

				case 3:
					sprintf(buf, "\nPower of radio in sensor was changed to -12 dBm.\n\r");
					break;

				case 4:
					sprintf(buf, "\nPower of radio in sensor was changed to -8 dBm.\n\r");
					break;

				case 5:
					sprintf(buf, "\nPower of radio in sensor was changed to -4 dBm.\n\r");
					break;

				case 6:
					sprintf(buf, "\nPower of radio in sensor was changed to 0 dBm.\n\r");
					break;

				case 7:
					sprintf(buf, "\nPower of radio in sensor was changed to +3 dBm.\n\r");
					break;

				case 8:
					sprintf(buf, "\nPower of radio in sensor was changed to +4 dBm.\n\r");
					break;
			}
			uartWriteS(buf);
		}
		else if( TRUE == testDone )
		{
			sprintf(buf, "\n\rWynik testu przy dlugosci pakietu 150 bajtow: \n\r");
			uartWriteS(buf);

			for(int i = 8; i < 16; i++)
			{
				sprintf(buf, "%f  ", resultsOfTests[i]);
				uartWriteS(buf);
			}

			uartWrite('\n');
			uartWrite('\r');

			doneTest = TRUE;
			turnOff = 1;
		}
		else
		{
			sprintf(buf, "\n\rWynik testu przy dlugosci pakietu 10 bajtow: \n\r");
			uartWriteS(buf);

			for(int i = 0; i < 8; i++)
			{
				sprintf(buf, "%f  ", resultsOfTests[i]);
				uartWriteS(buf);
			}

			uartWrite('\n');
			uartWrite('\r');

			txPower = 0;

			packetLength = 150;
			testDone = TRUE;
		}
	}
}
