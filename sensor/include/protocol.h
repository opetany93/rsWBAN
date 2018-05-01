#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "hal.h"
#include "ADXL362.h"
#include "radio.h"

#define SYNC									0xAA
#define ACK										0x55

#define ADVERTISEMENT_CHANNEL					30UL
#define FIRST_CHANNEL							0UL
#define SYNC_CHANNEL							29UL

#define LENGTH_PACKET_FOR_TESTS 				15

typedef enum 
{
	PACKET_data									= 0x00U,
  PACKET_init       							= 0xA5U,
} PACKET_type_t;

typedef struct{
	
	uint8_t			payloadSize;
	uint8_t 		sync;
	uint16_t 		rtc_val_CC0;
	uint16_t 		rtc_val_CC1;
	uint8_t			txPower;					//	value in dBm
	uint8_t			turnOff;
	
}sync_packet_t;

typedef struct{
	
	uint8_t			payloadSize;
	uint8_t 		ack;
	uint16_t 		rtc_val_CC0;
	uint32_t 		rtc_val_CC1;
	uint8_t 		channel;
	
}init_packet_t;

typedef struct{
	
	uint8_t			payloadSize;
	uint8_t 		type;
	uint8_t 		channel;
	ADXL362_AXES_t 	axes;
	//int								pressure;
	
	uint16_t 		data[15];
	
}data_packet_t;

// =================================== Functions ==========================================
int8_t connect(void);

void initProtocol(Radio *radioDrv);

void timeSlotHandler(void);
void syncHandler(void);
void radioSensorHandler(void);

void timeSlotCallback(void);

#endif
