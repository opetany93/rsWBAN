#ifndef __MYDEFINITIONS_H_
#define __MYDEFINITIONS_H_

#define RNG_ENABLE()						NRF_RNG->CONFIG = 1

#if defined(NRF52_SENSOR)

	// GPIO
	#define LED_1          					20
	#define LED_2          					18
	#define BUTTON							8
	#define ADC_PIN							28

	#define LED_1_ON()						NRF_GPIO->OUTCLR = (1 << LED_1)
	#define LED_2_ON()						NRF_GPIO->OUTCLR = (1 << LED_2)

	#define LED_1_OFF()						NRF_GPIO->OUTSET = (1 << LED_1)
	#define LED_2_OFF()						NRF_GPIO->OUTSET = (1 << LED_2)

	#define LED_1_TOGGLE()					NRF_GPIO->OUT ^= (1 << LED_1)
	#define LED_2_TOGGLE()					NRF_GPIO->OUT ^= (1 << LED_2)

	#define BUTTON_INTERRUPT_ENABLE()		GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Enabled << GPIOTE_INTENSET_PORT_Pos
	#define BUTTON_INTERRUPT_DISABLE()		GPIOTE->INTENCLR = GPIOTE_INTENCLR_PORT_Enabled << GPIOTE_INTENCLR_PORT_Pos

	// UART
	#define UART_TX_PIN						10
	#define UART_RX_PIN						9

	// I2C
	#define SCL_PIN							6
	#define SDA_PIN							7

	// SPI
	#define SCK_PIN							5
	#define MOSI_PIN						2
	#define MISO_PIN						3
	#define CS_PIN							4


	// ============================== Priorities of interrupts ===========================================
	#define RTC0_INTERRUPT_PRIORITY			1
	#define RADIO_INTERRUPT_PRIORITY		2
	#define TIMER0_INTERRUPT_PRIORITY		3
	#define GPIOTE_INTERRUPT_PRIORITY		4
	#define TIMER1_INTERRUPT_PRIORITY		5
	#define RTC1_INTERRUPT_PRIORITY			6

#elif defined(BOARD_PCA10040)

	#define LED_1          					17
	#define LED_2          					18
	#define LED_3          					19
	#define LED_4          					20
	
	#define BUTTON_1       					13
	#define BUTTON_2       					14
	#define BUTTON_3       					15
	#define BUTTON_4       					16

	#define UART_RX_PIN						8
	#define UART_TX_PIN						6

	#define LED_1_ON()						NRF_GPIO->OUTCLR = (1 << LED_1)
	#define LED_2_ON()						NRF_GPIO->OUTCLR = (1 << LED_2)
	#define LED_3_ON()						NRF_GPIO->OUTCLR = (1 << LED_3)
	#define LED_4_ON()						NRF_GPIO->OUTCLR = (1 << LED_4)

	#define LED_1_OFF()						NRF_GPIO->OUTSET = (1 << LED_1)
	#define LED_2_OFF()						NRF_GPIO->OUTSET = (1 << LED_2)
	#define LED_3_OFF()						NRF_GPIO->OUTSET = (1 << LED_3)
	#define LED_4_OFF()						NRF_GPIO->OUTSET = (1 << LED_4)

	#define LED_1_TOGGLE()					NRF_GPIO->OUT ^= (1 << LED_1)
	#define LED_2_TOGGLE()					NRF_GPIO->OUT ^= (1 << LED_2)
	#define LED_3_TOGGLE()					NRF_GPIO->OUT ^= (1 << LED_3)
	#define LED_4_TOGGLE()					NRF_GPIO->OUT ^= (1 << LED_4)

	// Arduino board mappings
	#define ARDUINO_SCL_PIN             	27    // SCL signal pin
	#define ARDUINO_SDA_PIN             	26    // SDA signal pin
	#define ARDUINO_AREF_PIN            	2     // Aref pin
	#define ARDUINO_13_PIN              	25    // Digital pin 13
	#define ARDUINO_12_PIN              	24    // Digital pin 12
	#define ARDUINO_11_PIN              	23    // Digital pin 11
	#define ARDUINO_10_PIN              	22    // Digital pin 10
	#define ARDUINO_9_PIN               	20    // Digital pin 9
	#define ARDUINO_8_PIN               	19    // Digital pin 8

	#define ARDUINO_7_PIN               	18    // Digital pin 7
	#define ARDUINO_6_PIN               	17    // Digital pin 6
	#define ARDUINO_5_PIN               	16    // Digital pin 5
	#define ARDUINO_4_PIN               	15    // Digital pin 4
	#define ARDUINO_3_PIN               	14    // Digital pin 3
	#define ARDUINO_2_PIN               	13    // Digital pin 2
	#define ARDUINO_1_PIN               	12    // Digital pin 1
	#define ARDUINO_0_PIN               	11    // Digital pin 0

	#define ARDUINO_A0_PIN              	3     // Analog channel 0
	#define ARDUINO_A1_PIN              	4     // Analog channel 1
	#define ARDUINO_A2_PIN              	28    // Analog channel 2
	#define ARDUINO_A3_PIN              	29    // Analog channel 3
	#define ARDUINO_A4_PIN              	30    // Analog channel 4
	#define ARDUINO_A5_PIN              	31    // Analog channel 5

		// ===================== Priorities of interrupts (BOARD_PCA10040)==================================
	#define RTC0_IRQ_PRIORITY				0
	#define RTC1_IRQ_PRIORITY				1
	#define RADIO_IRQ_PRIORITY				2
	#define UARTE_IRQ_PRIORITY				3
	
#endif

// ============================== Peripheral declaration =============================================
#define PPI 								NRF_PPI
#define RTC0								NRF_RTC0
#define RTC1								NRF_RTC1
#define RTC2								NRF_RTC2
#define RADIO								NRF_RADIO
#define TIMER0								NRF_TIMER0
#define TIMER1								NRF_TIMER1
#define CLOCK								NRF_CLOCK
#define GPIOTE								NRF_GPIOTE
#define UART								NRF_UART0
#define TWI0 								NRF_TWI0
#define TWI1 								NRF_TWI1
#define SPI0								NRF_SPI0
#define SPI1								NRF_SPI1
#define SPI2								NRF_SPI2
#define GPIO								NRF_GPIO
#define UARTE								NRF_UARTE0
#define RNG									NRF_RNG

#endif /* __MYDEFINITIONS_H_ */
