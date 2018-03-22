#ifndef PROTOCOL_HOST_H_
#define PROTOCOL_HOST_H_

#include <stdint.h>
#include "ADXL362.h"

#define FREQ_COLLECT_DATA_100mHz					1
#define FREQ_COLLECT_DATA_1Hz						2
#define FREQ_COLLECT_DATA_10Hz						3
#define FREQ_COLLECT_DATA_20Hz						4

#define SYNC										0xAA
#define ACK											0x55

#define ADVERTISEMENT_CHANNEL						30UL
#define FIRST_CHANNEL								0UL
#define SYNC_CHANNEL								29UL

#define LENGTH_PACKET_FOR_TESTS 					30

typedef enum 
{
	PACKET_data					= 0x00U,
	PACKET_init       			= 0xA5U,
} PACKET_type_t;

typedef struct{
	
	uint8_t		payloadSize;
	uint8_t 	sync;
	uint16_t 	rtc_val_CC0;
	uint16_t 	rtc_val_CC1;
	uint8_t 	testPacketLength;		//	value in bytes
	uint8_t		txPower;				//	value in dBm
	uint8_t		turnOff;
	
}sync_packet_t;

typedef struct{
	
	uint8_t		payloadSize;
	uint8_t 	ack;
	uint16_t 	rtc_val_CC0;
	uint32_t 	rtc_val_CC1;
	uint8_t 	channel;
	
}init_packet_t;

typedef struct{
	
	uint8_t						payloadSize;
	uint8_t 					type;
	uint8_t 					channel;
	ADXL362_AXES_t 				axes;
	//int							pressure;
	
	uint16_t data[15];
	
}data_packet_t;

// =================================== Functions ==========================================
void startHFCLK(void);
void RTC0_Sync(void);
void RTC1_TimeSlot(void);
void setFreqCollectData(uint8_t freq);
void lcdProtocolInit(void);
void startListening(void);

void sendDataToPC(void);

void radioHostHandler(void);
void timeSlotListenerHandler(void);
void syncTransmitHandler(void);

uint32_t  rnd8(void);

#endif
