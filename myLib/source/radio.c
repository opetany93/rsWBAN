#include "radio.h"
#include "hal.h"
#include "mydefinitions.h"
#include "clocks.h"

uint8_t 			tempRSSI;

volatile uint8_t 	timeout_flag;

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
void radioSensorInit(void)
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

	NVIC_SetPriority(RADIO_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), RADIO_INTERRUPT_PRIORITY, 0));
	NVIC_EnableIRQ(RADIO_IRQn);
}

#elif defined(BOARD_PCA10040)
//=======================================================================================
void radioHostInit(void)
{
	startHFCLK();
	// Radio config
	NRF_RADIO->TXPOWER   = RADIO_TXPOWER_TXPOWER_Pos4dBm;
	NRF_RADIO->MODE      = RADIO_MODE_MODE_Nrf_1Mbit;

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
}
#endif
// =======================================================================================
void sendPacket(uint32_t *packet)
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
int8_t readPacket(uint32_t *packet)
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
static void TIMER0_init(void)
{
	//24bit mode
	TIMER0->BITMODE = TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos;
	//Enable interrupt for COMPARE[0]
	TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled <<  TIMER_INTENSET_COMPARE0_Pos;
}

// =======================================================================================
static void TIMER0_deinit(void)
{
	TIMER0->TASKS_SHUTDOWN = 1U;
	TIMER0->INTENCLR = TIMER_INTENCLR_COMPARE0_Enabled <<  TIMER_INTENCLR_COMPARE0_Pos;
	
	//NVIC_DisableIRQ(TIMER0_IRQn);
}

// =======================================================================================
int8_t readPacketWithTimeout(uint32_t *packet, uint16_t timeout_ms)
{
	RADIO->PACKETPTR = (uint32_t)packet;
	
	TIMER0_init();
	TIMER0->CC[0] = timeout_ms * 1000;
	TIMER0->TASKS_START = 1U;
	
	//connect READY event to START task
	RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk;
	RADIO->EVENTS_READY = 0U;
	RADIO->EVENTS_END = 0U;
	RADIO->TASKS_RXEN = 1U;
	
	while (!(RADIO->EVENTS_END || timeout_flag));
	
	TIMER0_deinit();
	
	if(timeout_flag)
	{
		timeout_flag = 0;
		return -2;
	}
	if (RADIO->CRCSTATUS == 1U) return 0;
	else return -1;
}

// =======================================================================================
int8_t sendPacketWithResponse(uint32_t *packet, uint16_t timeout_ms)
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
void disableRadio(void)
{
	RADIO->EVENTS_DISABLED = 0U;
	RADIO->TASKS_DISABLE = 1U;						// disable radio
	while (RADIO->EVENTS_DISABLED == 0U);			// wait as long as radio will be disabled
	RADIO->EVENTS_DISABLED = 0;
}

void timeoutInterruptHandler(void)
{
	timeout_flag = 1;
		
	//timer stop
	//TIMER0->TASKS_STOP = 1U;
	TIMER0->TASKS_CLEAR = 1U;
}
