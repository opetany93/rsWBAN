#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "radio.h"
#include "rtc.h"
#include "packets.h"

typedef enum
{
	CONNECTED				= 0x00U,
	CRCError      			= 0x01U,
	RESPONSE_TIMEOUT   		= 0x02U,
	DISCONNECTED			= 0x03U,
	WRONG_PACKET_TYPE		= 0x04U,
	ALREADY_CONNECTED		= 0x05U

} protocol_status_t;

typedef struct{

	void (*timeSlotCallback)(data_packet_t* dataPacketPtr);
	void (*syncCallback)(void);

}SensorCallbacks_t;

// =================================== Functions ==========================================
protocol_status_t connect(void);

void initProtocol(Radio* radioDrv, Rtc* rtcDrv, SensorCallbacks_t this);
void deInitProtocol(void);

void timeSlotHandler(void);
void syncHandler(void);
void radioSensorHandler(void);

#endif
