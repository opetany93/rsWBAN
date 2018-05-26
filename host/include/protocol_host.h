#ifndef PROTOCOL_HOST_H_
#define PROTOCOL_HOST_H_

#include <stdint.h>
#include "radio.h"
#include "rtc.h"
#include "ADXL362.h"

#include "packets.h"

#define FREQ_COLLECT_DATA_100mHz					1
#define FREQ_COLLECT_DATA_1Hz						2
#define FREQ_COLLECT_DATA_10Hz						3
#define FREQ_COLLECT_DATA_20Hz						4

#define BROADCAST_CHANNEL						30UL
#define FIRST_CHANNEL								0UL
#define SYNC_CHANNEL								29UL

#define LENGTH_PACKET_FOR_TESTS 					30

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

	void (*start)(void);
	void (*setFreqCollectData)(uint8_t freq);

}Protocol;

// =================================== Functions ==========================================
Protocol* initProtocol(Radio* radioDrv, Rtc* rtc0Drv, Rtc* rtc1Drv);

void lcdProtocolInit(void);

void dataReadyCallback(data_packet_t** packets, uint8_t amountOfConnectedSensors);

void radioHostHandler(void);
void timeSlotListenerHandler(void);
void startTimeSlotListener(void);
void syncTransmitHandler(void);

#endif
