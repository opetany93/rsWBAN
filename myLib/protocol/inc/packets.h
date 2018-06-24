#ifndef PACKETS_H_
#define PACKETS_H_

#include "stdint.h"
#include "stdbool.h"

typedef struct{
	
	uint8_t 	packetType;
	uint8_t		payloadSize;
	uint16_t 	rtcValCC0;
	uint16_t 	rtcValCC1;
	uint8_t		txPower;				//	value in dBm
	uint8_t		approvals;
	uint8_t		turnOff;
	uint8_t 	startMes;
	
}sync_packet_t;

typedef struct{
	
	uint8_t 		packetType;
	uint8_t			payloadSize;
	uint16_t 		rtcValCC0;
	uint32_t 		rtcValCC1;
	uint8_t 		channel;
	
}init_packet_t;

typedef struct{
	
	uint8_t 		packetType;
	uint8_t			payloadSize;
	uint8_t			numberOfPacket;
	bool			disconnect;
	uint8_t 		channel;
	//int								pressure;
	
	uint8_t 		data[5][6];
	
}data_packet_t;

typedef enum
{
	PACKET_data									= 0x00U,
	PACKET_init       							= 0x55U,
	PACKET_sync									= 0xAAU

} PACKET_type_t;

//=======================================================================================
bool isPacketSync(sync_packet_t* packetPtr);
bool checkApprovals(sync_packet_t* packetPtr, uint8_t channel);
bool isPacketInit(init_packet_t* packetPtr);
bool isPacketData(data_packet_t* packetPtr);


#endif
