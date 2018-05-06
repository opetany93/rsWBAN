#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "hal.h"
#include "ADXL362.h"
#include "radio.h"

#define ADVERTISEMENT_CHANNEL					30UL
#define FIRST_CHANNEL							0UL
#define SYNC_CHANNEL							29UL

#define LENGTH_PACKET_FOR_TESTS 				15

typedef enum 
{
	PACKET_data									= 0x00U,
	PACKET_init       							= 0x55U,
	PACKET_sync									= 0xAAU

} PACKET_type_t;

typedef struct{
	
	uint8_t				payloadSize;
	uint8_t 			packetType;
	uint16_t 			rtc_val_CC0;
	uint16_t 			rtc_val_CC1;
	uint8_t				txPower;					//	value in dBm
	uint8_t				turnOff;
	
}sync_packet_t;

typedef struct{
	
	uint8_t			payloadSize;
	uint8_t 		packetType;
	uint16_t 		rtc_val_CC0;
	uint32_t 		rtc_val_CC1;
	uint8_t 		channel;
	
}init_packet_t;

typedef struct{
	
	uint8_t			payloadSize;
	uint8_t 		packetType;
	uint8_t 		channel;
	ADXL362_AXES_t 	axes;
	//int								pressure;
	
	uint16_t 		data[15];
	
}data_packet_t;

typedef enum
{
	CONNECTED				= 0x00U,
	CRCError      			= 0x01U,
	RESPONSE_TIMEOUT   		= 0x02U,
	DISCONNECTED			= 0x03U,
	WRONG_PACKET_TYPE		= 0x04U,
	ALREADY_CONNECTED		= 0x05U

} connect_status_t;

// =================================== Functions ==========================================
connect_status_t connect(void);

void initProtocol(Radio *radioDrv);
void deInitProtocol();

void timeSlotHandler(void);
void syncHandler(void);
void radioSensorHandler(void);

void timeSlotCallback(void);

#endif
