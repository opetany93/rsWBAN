#ifndef __RADIO_H_
#define __RADIO_H_

#include <stdint.h>
#include <stdbool.h>

#define RADIO_DISABLE()				RADIO->TASKS_DISABLE = 1U

#define PACKET_SIZE					255		// value in bytes

/* These are set to zero as Shockburst packets don't have corresponding fields. */
#define PACKET_S1_FIELD_SIZE      	(8UL)  	/**< Packet S1 field size in bits. */
#define PACKET_S0_FIELD_SIZE      	(1UL)  	/**< Packet S0 field size in bytes. */
#define PACKET_LENGTH_FIELD_SIZE  	(8UL)  	/**< Packet length field size in bits. */

#define PACKET_BASE_ADDRESS_LENGTH  (4UL)   //!< Packet base address length field size in bytes
#define PACKET_STATIC_LENGTH        (0)     //!< Packet static length in bytes
#define PACKET_PAYLOAD_MAXSIZE      (255UL) //!< Packet payload maximum size in bytes

// =======================================================================================
typedef enum 
{
	RADIO_OK,
	RADIO_CRCError,
	RADIO_ACKError,
	RADIO_TIMEOUTError,
	RADIO_NOT_OK		

} RADIO_status_t;

typedef struct{

	void (*sendPacket)(uint32_t *packet);
	int8_t (*readPacket)(uint32_t *packet);
	RADIO_status_t (*readPacketWithTimeout)(uint32_t *packet, uint16_t timeout_ms);
	RADIO_status_t (*sendPacketWithResponse)(uint32_t *packet, uint16_t timeout_ms);
	void (*disableRadio)(void);
	bool (*isRxIdleState)(void);
	bool (*checkCRC)(void);
	void (*setChannel)(uint8_t channel);
	void (*setPacketPtr)(uint32_t ptr);
	void (*rxEnable)(void);
	void (*txEnable)(void);
	void (*endInterruptEnable)(void);
	void (*endInterruptDisable)(void);
	void (*clearFlags)(void);
	void (*readyToStartShortcutSet)(void);
	void (*endToDisableShortcutSet)(void);
	void (*endToDisableShortcutUnset)(void);

}Radio;

// =======================================================================================

#if defined(NRF52_SENSOR)

Radio* radioSensorInit(void);

#elif defined(NRF52_HOST)

Radio* radioHostInit(void);

#endif

uint8_t getRSSI(void);

void timeoutInterruptHandler(void);

#endif
