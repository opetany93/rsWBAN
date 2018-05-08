#include "radio.h"
#include "hal.h"
#include "mydefinitions.h"
#include "clocks.h"
#include "nrf.h"

#include <stdlib.h>

uint8_t tempRSSI;

static inline uint8_t isRxIdleState(void);
static inline uint8_t checkCRC(void);
static inline void setChannel(uint8_t channel);
static inline void setPacketPtr(uint32_t ptr);
static inline void sendPacket(uint32_t *packet);
static inline void rxEnable(void);
static inline void txEnable(void);
static inline void disableRadio(void);
static inline void endInterruptEnable(void);
static inline void endInterruptDisable(void);
static inline void clearFlags(void);
static inline void readyToStartShortcutSet(void);
static inline void endToDisableShortcutSet(void);
static inline void endToDisableShortcutUnset(void);

static inline int8_t readPacket(uint32_t *packet);
static inline uint8_t readPacketWithTimeout(uint32_t *packet, uint16_t timeout_ms);
static inline uint8_t sendPacketWithResponse(uint32_t *packet, uint16_t timeout_ms);

static volatile uint8_t timeoutFlag = 0;

/**
 * @brief Function for swapping/mirroring bits in a byte.
 * 
 *@verbatim
 * output_bit_7 = input_bit_0
 * output_bit_6 = input_bit_1
 *           :
 * output_bit_0 = input_bit_7
 *@endverbatim
 *
 * @param[in] inp is the input byte to be swapped.
 *
 * @return
 * Returns the swapped/mirrored input byte.
 */
static uint32_t swap_bits(uint32_t inp)
{
	uint32_t i;
	uint32_t retval = 0;

	inp = (inp & 0x000000FFUL);

	for (i = 0; i < 8; i++)
	{
		retval |= ((inp >> i) & 0x01) << (7 - i);
	}

	return retval;    
}

/**
 * @brief Function for swapping bits in a 32 bit word for each byte individually.
 * 
 * The bits are swapped as follows:
 * @verbatim
 * output[31:24] = input[24:31] 
 * output[23:16] = input[16:23]
 * output[15:8]  = input[8:15]
 * output[7:0]   = input[0:7]
 * @endverbatim
 * @param[in] input is the input word to be swapped.
 *
 * @return
 * Returns the swapped input byte.
 */
static uint32_t bytewise_bitswap(uint32_t inp)
{
	return (swap_bits(inp >> 24) << 24)
		 | (swap_bits(inp >> 16) << 16)
	     | (swap_bits(inp >> 8) << 8)
		 | (swap_bits(inp));
}

#if defined(NRF52_SENSOR)
// =======================================================================================
static inline void TIMER0_init(void)
{
	//set NVIC for TIMER0 which is used for timeout in function ReadPacketWithTimeout
	NVIC_SetPriority(TIMER0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), MIDDLE_IRQ_PRIO, TIMER0_INTERRUPT_PRIORITY));
	NVIC_EnableIRQ(TIMER0_IRQn);

	//24bit mode
	TIMER0->BITMODE = TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos;
	//Enable interrupt for COMPARE[0]
	TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled <<  TIMER_INTENSET_COMPARE0_Pos;

	TIMER0->TASKS_STOP = 1U;
	TIMER0->TASKS_CLEAR = 1U;

	TIMER0->SHORTS = TIMER_SHORTS_COMPARE0_STOP_Msk;
}

// =======================================================================================
Radio* radioSensorInit(void)
{
	// Radio config
	RADIO->TXPOWER   = RADIO_TXPOWER_TXPOWER_0dBm;
	//RADIO->FREQUENCY = ADVERTISEMENT_CHANNEL; 							// Frequency bin 30, 2430MHz
	RADIO->MODE      = RADIO_MODE_MODE_Nrf_1Mbit;

	// Radio address config
	RADIO->PREFIX0 = ((uint32_t)swap_bits(0xC3) << 24) // Prefix byte of address 3 converted to nRF24L series format
					|((uint32_t)swap_bits(0xC2) << 16) // Prefix byte of address 2 converted to nRF24L series format
					|((uint32_t)swap_bits(0xC1) << 8)  // Prefix byte of address 1 converted to nRF24L series format
					|((uint32_t)swap_bits(0xC0) << 0); // Prefix byte of address 0 converted to nRF24L series format

	RADIO->PREFIX1 = ((uint32_t)swap_bits(0xC7) << 24) // Prefix byte of address 7 converted to nRF24L series format
					|((uint32_t)swap_bits(0xC6) << 16) // Prefix byte of address 6 converted to nRF24L series format
					|((uint32_t)swap_bits(0xC4) << 0); // Prefix byte of address 4 converted to nRF24L series format

	RADIO->BASE0 = bytewise_bitswap(0x01234567UL);  // Base address for prefix 0 converted to nRF24L series format
	RADIO->BASE1 = bytewise_bitswap(0x89ABCDEFUL);  // Base address for prefix 1-7 converted to nRF24L series format

	RADIO->TXADDRESS   = 0x00UL;  // Set device address 0 to use when transmitting
	RADIO->RXADDRESSES = 0x01UL;  // Enable device address 0 to use to select which addresses to receive

	// Packet configuration
	RADIO->PCNF0 = (PACKET_S1_FIELD_SIZE     << RADIO_PCNF0_S1LEN_Pos) |
				   (PACKET_S0_FIELD_SIZE     << RADIO_PCNF0_S0LEN_Pos) |
				   (PACKET_LENGTH_FIELD_SIZE << RADIO_PCNF0_LFLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

	// Packet configuration
	RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
				   (RADIO_PCNF1_ENDIAN_Big       << RADIO_PCNF1_ENDIAN_Pos)  |
				   (PACKET_BASE_ADDRESS_LENGTH   << RADIO_PCNF1_BALEN_Pos)   |
				   (PACKET_STATIC_LENGTH         << RADIO_PCNF1_STATLEN_Pos) |
				   (PACKET_PAYLOAD_MAXSIZE       << RADIO_PCNF1_MAXLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

	// CRC Config
	RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // Number of checksum bits
	
	RADIO->CRCINIT = 0xFFFFUL;   // Initial value
	RADIO->CRCPOLY = 0x11021UL;  // CRC poly: x^16+x^12^x^5+1

	NVIC_SetPriority(RADIO_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), HIGH_IRQ_PRIO, RADIO_INTERRUPT_PRIORITY));
	NVIC_EnableIRQ(RADIO_IRQn);

	Radio *radio = malloc(sizeof(Radio));
	radio->sendPacket = sendPacket;
	radio->readPacket = readPacket;
	radio->readPacketWithTimeout = readPacketWithTimeout;
	radio->sendPacketWithResponse = sendPacketWithResponse;
	radio->disableRadio = disableRadio;
	radio->isRxIdleState = isRxIdleState;
	radio->checkCRC = checkCRC;
	radio->setChannel = setChannel;
	radio->setPacketPtr = setPacketPtr;
	radio->rxEnable = rxEnable;
	radio->txEnable = txEnable;
	radio->endInterruptEnable = endInterruptEnable;
	radio->endInterruptDisable = endInterruptDisable;
	radio->clearFlags = clearFlags;
	radio->readyToStartShortcutSet = readyToStartShortcutSet;
	radio->endToDisableShortcutSet = endToDisableShortcutSet;
	radio->endToDisableShortcutUnset = endToDisableShortcutUnset;

	TIMER0_init();

	return radio;
}

#elif defined(BOARD_PCA10040)
//=======================================================================================
Radio* radioHostInit(void)
{
	startHFCLK();
	// Radio config
	NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Pos4dBm;
	NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_1Mbit;

	// Radio address config
	NRF_RADIO->PREFIX0 = 
			((uint32_t)swap_bits(0xC3) << 24) // Prefix byte of address 3 converted to nRF24L series format
		  | ((uint32_t)swap_bits(0xC2) << 16) // Prefix byte of address 2 converted to nRF24L series format
		  | ((uint32_t)swap_bits(0xC1) << 8)  // Prefix byte of address 1 converted to nRF24L series format
		  | ((uint32_t)swap_bits(0xC0) << 0); // Prefix byte of address 0 converted to nRF24L series format

	NRF_RADIO->PREFIX1 = 
			((uint32_t)swap_bits(0xC7) << 24) // Prefix byte of address 7 converted to nRF24L series format
		  | ((uint32_t)swap_bits(0xC6) << 16) // Prefix byte of address 6 converted to nRF24L series format
		  | ((uint32_t)swap_bits(0xC4) << 0); // Prefix byte of address 4 converted to nRF24L series format

	NRF_RADIO->BASE0 = bytewise_bitswap(0x01234567UL);  // Base address for prefix 0 converted to nRF24L series format
	NRF_RADIO->BASE1 = bytewise_bitswap(0x89ABCDEFUL);  // Base address for prefix 1-7 converted to nRF24L series format

	NRF_RADIO->TXADDRESS   = 0x00UL;  // Set device address 0 to use when transmitting
	NRF_RADIO->RXADDRESSES = 0x01UL;  // Enable device address 0 to use to select which addresses to receive

	// Packet configuration
	NRF_RADIO->PCNF0 = (PACKET_S1_FIELD_SIZE     << RADIO_PCNF0_S1LEN_Pos) |
					   (PACKET_S0_FIELD_SIZE     << RADIO_PCNF0_S0LEN_Pos) |
					   (PACKET_LENGTH_FIELD_SIZE << RADIO_PCNF0_LFLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

	// Packet configuration
	NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
					   (RADIO_PCNF1_ENDIAN_Big       << RADIO_PCNF1_ENDIAN_Pos)  |
					   (PACKET_BASE_ADDRESS_LENGTH   << RADIO_PCNF1_BALEN_Pos)   |
					   (PACKET_STATIC_LENGTH         << RADIO_PCNF1_STATLEN_Pos) |
					   (PACKET_PAYLOAD_MAXSIZE       << RADIO_PCNF1_MAXLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

	// CRC Config
	NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // Number of checksum bits

	NRF_RADIO->CRCINIT = 0xFFFFUL;   // Initial value
	NRF_RADIO->CRCPOLY = 0x11021UL;  // CRC poly: x^16+x^12^x^5+1
	
	NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos;
	NRF_RADIO->INTENSET = RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos;
	
	//set NVIC
	NVIC_SetPriority(RADIO_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), LOW_IRQ_PRIO, RADIO_IRQ_PRIORITY));
	NVIC_EnableIRQ(RADIO_IRQn);

	Radio *radio = malloc(sizeof(Radio));
	radio->sendPacket = sendPacket;
	radio->readPacket = readPacket;
	radio->readPacketWithTimeout = readPacketWithTimeout;
	radio->sendPacketWithResponse = sendPacketWithResponse;
	radio->disableRadio = disableRadio;
	radio->isRxIdleState = isRxIdleState;
	radio->checkCRC = checkCRC;
	radio->setChannel = setChannel;
	radio->setPacketPtr = setPacketPtr;
	radio->rxEnable = rxEnable;
	radio->endInterruptEnable = endInterruptEnable;
	radio->endInterruptDisable = endInterruptDisable;
	radio->clearFlags = clearFlags;
	radio->readyToStartShortcutSet = readyToStartShortcutSet;
	radio->endToDisableShortcutSet = endToDisableShortcutSet;
	radio->endToDisableShortcutUnset = endToDisableShortcutUnset;

	return radio;
}
#endif
// =======================================================================================
static inline void sendPacket(uint32_t *packet)
{
	RADIO->PACKETPTR = (uint32_t)packet;
	//connect READY event to START task
	RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk;
	RADIO->EVENTS_READY = 0U;
	RADIO->EVENTS_END = 0U;
	RADIO->TASKS_TXEN = 1U;
	while (!RADIO->EVENTS_END);
}

// TODO function readPacket with getRSSI

// =======================================================================================
static inline int8_t readPacket(uint32_t *packet)
{
	RADIO->PACKETPTR = (uint32_t)packet;
	//connect READY event to START task
	RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk;
	RADIO->EVENTS_READY = 0U;
	RADIO->EVENTS_END = 0U;
	RADIO->TASKS_RXEN = 1U;
	while (!RADIO->EVENTS_END);
	
	if (RADIO->CRCSTATUS == 1U) return 0;
	else return -1;
}

// =======================================================================================
static inline uint8_t readPacketWithTimeout(uint32_t *packet, uint16_t timeout_ms)
{
	RADIO->PACKETPTR = (uint32_t)packet;
	
	TIMER0->CC[0] = timeout_ms * 1000;
	TIMER0->TASKS_CLEAR = 1U;
	
	//connect READY event to START task
	RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk;
	clearFlags();
	RADIO->TASKS_RXEN = 1U;
	TIMER0->TASKS_START = 1U;
	
	while ((0 == RADIO->EVENTS_END) && (0 == timeoutFlag))
		;
	
	RADIO->EVENTS_END = 0;
	
	if(timeoutFlag)
	{
		timeoutFlag = 0;
		return RADIO_TIMEOUTError;
	}
	if (RADIO->CRCSTATUS == 1U) return RADIO_OK;
	else return RADIO_CRCError;
}

// =======================================================================================
static inline uint8_t sendPacketWithResponse(uint32_t *packet, uint16_t timeout_ms)
{
	sendPacket((uint32_t *)packet);

	return readPacketWithTimeout((uint32_t *)packet, timeout_ms);
}

// =======================================================================================
uint8_t getRSSI(void)
{
	uint8_t rssi;
	
	RADIO->TASKS_RSSISTART = 1;;
	while(!(RADIO->EVENTS_RSSIEND));
	rssi = (uint8_t)RADIO->RSSISAMPLE;
	RADIO->TASKS_RSSISTOP = 1;
	
	return rssi;
}

// =======================================================================================
static inline void disableRadio(void)
{
	RADIO->EVENTS_DISABLED = 0U;
	RADIO->TASKS_DISABLE = 1U;						// disable radio
	while (RADIO->EVENTS_DISABLED == 0U);			// wait as long as radio will be disabled
	RADIO->EVENTS_DISABLED = 0;
}

inline void timeoutInterruptHandler(void)
{
	timeoutFlag = 1;
	TIMER1->TASKS_STOP = 1U;
}

static inline uint8_t isRxIdleState()
{
	return RADIO->STATE == RADIO_STATE_STATE_RxIdle;
}

static inline uint8_t checkCRC()
{
	return RADIO->CRCSTATUS == 1U;
}

static inline void setChannel(uint8_t channel)
{
	RADIO->FREQUENCY = channel;
}

static inline void setPacketPtr(uint32_t ptr)
{
	RADIO->PACKETPTR = ptr;
}

static inline void rxEnable()
{
	RADIO->TASKS_RXEN = 1U;
}

static inline void txEnable()
{
	RADIO->TASKS_TXEN = 1U;
}

static inline void endInterruptEnable()
{
	RADIO->INTENSET = (RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos);
}

static inline void endInterruptDisable()
{
	RADIO->INTENCLR = (RADIO_INTENCLR_END_Enabled << RADIO_INTENCLR_END_Pos);
}

static inline void clearFlags()
{
	RADIO->EVENTS_READY = 0U;
	RADIO->EVENTS_END = 0U;
}

static inline void readyToStartShortcutSet(void)
{
	RADIO->SHORTS |= RADIO_SHORTS_READY_START_Msk;
}
static inline void endToDisableShortcutSet(void)
{
	RADIO->SHORTS |= RADIO_SHORTS_END_DISABLE_Msk;
}
static inline void endToDisableShortcutUnset(void)
{
	RADIO->SHORTS &= ~RADIO_SHORTS_END_DISABLE_Msk;
}

