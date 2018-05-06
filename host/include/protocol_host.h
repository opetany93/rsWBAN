#ifndef PROTOCOL_HOST_H_
#define PROTOCOL_HOST_H_

#include <stdint.h>
#include "radio.h"
#include "ADXL362.h"

#define FREQ_COLLECT_DATA_100mHz					1
#define FREQ_COLLECT_DATA_1Hz						2
#define FREQ_COLLECT_DATA_10Hz						3
#define FREQ_COLLECT_DATA_20Hz						4

#define ADVERTISEMENT_CHANNEL						30UL
#define FIRST_CHANNEL								0UL
#define SYNC_CHANNEL								29UL

#define LENGTH_PACKET_FOR_TESTS 					30

typedef enum 
{
	PACKET_data									= 0x00U,
	PACKET_init       							= 0x55U,
	PACKET_sync									= 0xAAU

} PACKET_type_t;

typedef struct{
	
	uint8_t		payloadSize;
	uint8_t 	packetType;
	uint16_t 	rtc_val_CC0;
	uint16_t 	rtc_val_CC1;
	uint8_t		txPower;				//	value in dBm
	uint8_t		turnOff;
	
}sync_packet_t;

typedef struct{
	
	uint8_t		payloadSize;
	uint8_t 	packetType;
	uint16_t 	rtc_val_CC0;
	uint32_t 	rtc_val_CC1;
	uint8_t 	channel;
	
}init_packet_t;

typedef struct{
	
	uint8_t						payloadSize;
	uint8_t 					packetType;
	uint8_t 					channel;
	ADXL362_AXES_t 				axes;
	//int							pressure;
	
	uint16_t data[15];
	
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

typedef struct{

	void (*startListening)(void);
	void (*setFreqCollectData)(uint8_t freq);

}Protocol;

// =================================== Functions ==========================================
Protocol* initProtocol(Radio* radioDrv);

void lcdProtocolInit(void);

void dataReadyCallback(data_packet_t** packets, uint8_t amountOfConnectedSensors);

void radioHostHandler(void);
void timeSlotListenerHandler(void);
void startTimeSlotListener(void);
void syncTransmitHandler(void);

#endif
