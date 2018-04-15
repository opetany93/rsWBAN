#ifndef __RADIO_H_
#define __RADIO_H_

#include "nrf.h"

#define RADIO_DISABLE()				RADIO->TASKS_DISABLE = 1U

#define RADIO_END_INT_ENABLE()		RADIO->INTENSET = (RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos)
#define RADIO_END_INT_DISABLE()		RADIO->INTENCLR = (RADIO_INTENCLR_END_Enabled << RADIO_INTENCLR_END_Pos)

#define PACKET_SIZE					255		// value in bytes

/* These are set to zero as Shockburst packets don't have corresponding fields. */
#define PACKET_S1_FIELD_SIZE      	(0UL)  	/**< Packet S1 field size in bits. */
#define PACKET_S0_FIELD_SIZE      	(0UL)  	/**< Packet S0 field size in bits. */
#define PACKET_LENGTH_FIELD_SIZE  	(8UL)  	/**< Packet length field size in bits. */

#define PACKET_BASE_ADDRESS_LENGTH  (4UL)   //!< Packet base address length field size in bytes
#define PACKET_STATIC_LENGTH        (0)     //!< Packet static length in bytes
#define PACKET_PAYLOAD_MAXSIZE      (255UL) //!< Packet payload maximum size in bytes

typedef struct{

	void (*sendPacket)(uint32_t *packet);
	int8_t (*readPacket)(uint32_t *packet);
	int8_t (*readPacketWithTimeout)(uint32_t *packet, uint16_t timeout_ms);
	int8_t (*sendPacketWithResponse)(uint32_t *packet, uint16_t timeout_ms);
	void (*disableRadio)(void);
	uint8_t (*isRxIdleState)(void);
	uint8_t (*checkCRC)(void);

}Radio;

// =======================================================================================
typedef enum 
{
	RADIO_OK			= 0x00U,
	RADIO_CRCError      = 0x01U,
	RADIO_ACKError   	= 0x02U,

} RADIO_status_t;

// =======================================================================================

#if defined(NRF52_SENSOR)

Radio* radioSensorInit(void);

#elif defined(NRF52_HOST)

Radio* radioHostInit(void);

#endif

uint8_t getRSSI(void);

void timeoutInterruptHandler(void);

#endif
