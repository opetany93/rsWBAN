#ifndef PACKETS_STRUCTURES_H_
#define PACKETS_STRUCTURES_H_

typedef struct{
	
	uint8_t		payloadSize;
	uint8_t 	packetType;
	uint16_t 	rtcValCC0;
	uint16_t 	rtcValCC1;
	uint8_t		txPower;				//	value in dBm
	uint8_t		approvals;
	uint8_t		turnOff;
	
}sync_packet_t;

typedef struct{
	
	uint8_t			payloadSize;
	uint8_t 		packetType;
	uint16_t 		rtcValCC0;
	uint32_t 		rtcValCC1;
	uint8_t 		channel;
	
}init_packet_t;

typedef struct{
	
	uint8_t			payloadSize;
	uint8_t 		packetType;
	uint8_t			numberOfPacket;
	uint8_t			disconnect;
	uint8_t 		channel;
	//int								pressure;
	
	uint8_t 		data[6];
	
}data_packet_t;

#endif
