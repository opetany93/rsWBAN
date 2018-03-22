#include "spi.h"

#include "mydefinitions.h"
#include <stdlib.h>

#define CS_HIGH				1
#define CS_LOW				0

//======================================================================================
uint8_t spiRead(spiHandle_t handle)
{
	if (NULL != handle)
	{
		if(!handle->isSended)
		{
			while(!(handle->SPIx->EVENTS_READY));
			handle->SPIx->EVENTS_READY = 0U;
			
			handle->isSended = 0;
		}
		
		return handle->SPIx->RXD;
	}
	else
		return 0; // TODO: return -1
}

//======================================================================================
void spiSend(spiHandle_t handle, uint8_t byte)
{
	if (NULL != handle)
	{
#if defined(NRF52_HOST)
		SPI0->TXD = byte;								//send byte
		SPI0->RXD;										//odczytuje rejestr RXD, inaczej sie zapetla program,
		while(!(NRF_SPI0->EVENTS_READY));
		SPI0->EVENTS_READY = 0x00;
#else
		handle->SPIx->TXD = byte;						//send byte
		handle->SPIx->RXD;								//odczytuje rejestr RXD, inaczej sie zapetla program,

		while(!(handle->SPIx->EVENTS_READY));
		handle->SPIx->EVENTS_READY = 0U;
#endif
		handle->isSended = 1;
	}
}

//======================================================================================
void spiControlChipSelect(spiHandle_t handle, uint8_t state)
{
	if (NULL != handle)
	{
		if(CS_HIGH == state) GPIO->OUTSET = (1 << handle->pinCS);
		else if(CS_LOW == state) GPIO->OUTCLR = (1 << handle->pinCS);
	}
}

//when MOSI or MISO is unused then equal NONE
//======================================================================================
spiHandle_t spiInit(NRF_SPI_Type* SPIx, uint8_t SCK, int8_t MOSI, int8_t MISO, uint8_t CS, uint32_t frequency, uint8_t mode, uint8_t order)
{
	spiHandle_t handle = malloc((unsigned int)sizeof(spiHandle_t));

	handle->SPIx = SPIx;
	handle->pinCS = CS;

	//gpio config
	GPIO->PIN_CNF[SCK]  = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
	GPIO->PIN_CNF[MOSI] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
	GPIO->PIN_CNF[MISO] = (GPIO_PIN_CNF_DIR_Input  << GPIO_PIN_CNF_DIR_Pos);
	GPIO->PIN_CNF[CS]   = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);

	GPIO->OUTSET = (1 << handle->pinCS);

	//pins select
	if(MOSI < 0) SPIx->PSEL.MOSI = 0xFFFFFFFF;
	else SPIx->PSEL.MOSI = MOSI;

	if (MISO < 0) SPIx->PSEL.MISO = 0xFFFFFFFF;
	else SPIx->PSEL.MISO = MISO;

	SPIx->PSEL.SCK = SCK;

	//enable interrupt for ready event (TXD byte sent and RXD byte received)
	SPIx->INTENSET = (SPI_INTENSET_READY_Enabled << SPI_INTENSET_READY_Pos);

	//frequency
	SPIx->FREQUENCY = frequency;

	//ustawiam tryb oraz ktory bit ma byc pierwszy wyslany
	SPIx->CONFIG = mode | order;

	//spi enable
	SPI_ENABLE(SPIx);

	return handle;
}

