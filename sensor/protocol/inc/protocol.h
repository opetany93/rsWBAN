#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "radio.h"
#include "packetStructures.h"

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

typedef enum
{
	CONNECTED				= 0x00U,
	CRCError      			= 0x01U,
	RESPONSE_TIMEOUT   		= 0x02U,
	DISCONNECTED			= 0x03U,
	WRONG_PACKET_TYPE		= 0x04U,
	ALREADY_CONNECTED		= 0x05U

} protocol_status_t;

// =================================== Functions ==========================================
protocol_status_t connect(void);

void initProtocol(Radio *radioDrv);
void deInitProtocol();

void timeSlotHandler(void);
void syncHandler(void);
void radioSensorHandler(void);

void timeSlotCallback(data_packet_t* dataPacketPtr);

#endif
