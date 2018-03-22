#include "ADXL362.h"
#include <stddef.h>

#define WRITE_CMD								0x0A
#define READ_CMD								0x0B
#define READ_FIFO_CMD							0x0D

#define SPI_CS_HIGH								1
#define SPI_CS_LOW								0

static void 		(*spiControlCSpin)(uint8_t state) = NULL;
static void 		(*spiSendFunc)(uint8_t byte) = NULL;
static uint8_t 		(*spiReadFunc)(void) = NULL;

// ===============================================================================================
static uint8_t readRegister(uint8_t address)
{
	uint8_t buffer;
	
	spiControlCSpin(SPI_CS_LOW);
	
	(*spiSendFunc)(READ_CMD);
	(*spiSendFunc)(address);
	(*spiSendFunc)(0x00);							// send clock for transmission and read
	
	buffer = (*spiReadFunc)();									
	
	spiControlCSpin(SPI_CS_HIGH);
	
	return buffer;
}

// ===============================================================================================
static void writeRegister(uint8_t address, uint8_t value)
{
	spiControlCSpin(SPI_CS_LOW);
	
	(*spiSendFunc)(WRITE_CMD);
	(*spiSendFunc)(address);
	(*spiSendFunc)(value);
	
	spiControlCSpin(SPI_CS_HIGH);
}

// ===============================================================================================
static uint16_t readRegister_16bit(uint8_t address)
{
	uint16_t wholeValue;
	
	spiControlCSpin(SPI_CS_LOW);
	
	(*spiSendFunc)(READ_CMD);
	(*spiSendFunc)(address);
	
	(*spiSendFunc)(0x00);							// send clock for transmission and read
	wholeValue = (*spiReadFunc)();

	(*spiSendFunc)(0x00);							// send clock for transmission and read
	wholeValue |= ((*spiReadFunc)() << 8);

	spiControlCSpin(SPI_CS_HIGH);
	
	return wholeValue;
}

int8_t ADXL362_FifoInit(uint16_t samplesInFifo)
{
	uint8_t registerValue;

	if( 256 > samplesInFifo)
		writeRegister(ADXL362_FIFO_SAMPLES, samplesInFifo);
	else
	{
		writeRegister(ADXL362_FIFO_CONTROL,(1 << ADXL362_AH));
		writeRegister(ADXL362_FIFO_SAMPLES, (uint8_t)samplesInFifo);
	}

//	if ( (0x100 | readRegister(ADXL362_FIFO_SAMPLES)) !=  samplesInFifo )
//		return -1;

	registerValue = readRegister(ADXL362_FIFO_CONTROL);
	registerValue |= ADXL362_FIFO_STREAM_MODE;
	writeRegister(ADXL362_FIFO_CONTROL, registerValue); //TODO: sprawdzic czy dziala po tej poprawce

	registerValue = readRegister(ADXL362_FIFO_CONTROL);
	registerValue <<= 6;
	registerValue >>= 6;

	if( ADXL362_FIFO_STREAM_MODE != registerValue )
		return -1;

	return 0;
}

// ===============================================================================================
int8_t ADXL362_ReadFifo(uint16_t* buffer, uint8_t length)
{
	if(NULL == spiSendFunc)
		return -1;

	spiControlCSpin(SPI_CS_LOW);

	(*spiSendFunc)(READ_FIFO_CMD);
	
	for (uint8_t i = 0; i < length; i++)
	{
		(*spiSendFunc)(0x00);						// send clock for transmission and read
		*buffer  = (*spiReadFunc)();
		
		(*spiSendFunc)(0x00);
		*buffer |= (*spiReadFunc)() << 8;
		buffer++;
	}
	
	spiControlCSpin(SPI_CS_HIGH);

	return 0;
}

// ===============================================================================================
double ADXL362_ReadTemp(void)
{
	if((NULL == spiSendFunc) || (NULL == spiReadFunc))
		return -1;

	uint16_t value;
	uint8_t sign;
	
	value = readRegister_16bit(ADXL362_TEMP_L);
	sign = value >> 12;
	
	value &= 0x0FFF;
	
	if (sign == 0xFF) value = -value;
	
	return (double)(value - 350) * 0.065;
}

// ===============================================================================================
int16_t ADXL362_ReadX(void)
{
	if((NULL != spiSendFunc) || (NULL != spiReadFunc))
		return readRegister_16bit(ADXL362_XDATA_L);
	else
		return -1;
}

// ===============================================================================================
int16_t ADXL362_ReadY(void)
{
	if((NULL != spiSendFunc) || (NULL != spiReadFunc))
		return readRegister_16bit(ADXL362_YDATA_L);
	else
		return -1;
}

// ===============================================================================================
int16_t ADXL362_ReadZ(void)
{
	if((NULL != spiSendFunc) || (NULL != spiReadFunc))
		return readRegister_16bit(ADXL362_ZDATA_L);
	else
		return -1;
}

// ===============================================================================================
int8_t ADXL362_ReadXYZ(ADXL362_AXES_t* axes)
{
	uint8_t* buf = (uint8_t*)axes;
	
	if((NULL == spiSendFunc) || (NULL == spiReadFunc))
		return -1;

	spiControlCSpin(SPI_CS_LOW);
	
	(*spiSendFunc)(READ_CMD);
	(*spiSendFunc)(ADXL362_XDATA_L);
	
	for (uint8_t i = 0; i < 6; i++)
	{
		(*spiSendFunc)(0x00);											// send clock for transmission and read
		*buf = (*spiReadFunc)();	//spiSendRecv(chosenSPI, 0x00);
		buf++;
	}
	
	spiControlCSpin(SPI_CS_HIGH);

	return 0;
}

// ===============================================================================================
int8_t ADXL362_Init(void (*spiSendPtr)(uint8_t byte), uint8_t (*spiReadPtr)(void), void (*spiControlChipSelect)(uint8_t state))
{
	spiReadFunc = spiReadPtr;
	spiSendFunc = spiSendPtr;
	spiControlCSpin = spiControlChipSelect;

	if((NULL == spiSendFunc) || (NULL == spiReadFunc) || (NULL == spiControlCSpin))
		return -1;
	
	writeRegister(ADXL362_POWER_CTL, (1 << ADXL362_MEASURE));
	
	if ( (1 << ADXL362_MEASURE) != readRegister(ADXL362_POWER_CTL) )
		return -1;
	
	return 0;
}
