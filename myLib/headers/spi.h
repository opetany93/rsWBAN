#ifndef __SPI_H_
#define __SPI_H_

#include "nrf.h"
#include <stdint.h>

struct spiHandle
{
	uint8_t pinCS;
	uint8_t isSended;
	NRF_SPI_Type* SPIx;
};

typedef struct spiHandle* spiHandle_t;

// ========================================================================================
#define SPI_ENABLE(SPIx)				SPIx->ENABLE = (SPI_ENABLE_ENABLE_Enabled  << SPI_ENABLE_ENABLE_Pos)
#define SPI_DISABLE(SPIx)				SPIx->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos)


// =================================== Frequencies ========================================
#define SPI_FREQUENCY_K125 				SPI_FREQUENCY_FREQUENCY_K125 	/*!< 125 kbps */
#define SPI_FREQUENCY_K250 				SPI_FREQUENCY_FREQUENCY_K250 	/*!< 250 kbps */
#define SPI_FREQUENCY_K500 				SPI_FREQUENCY_FREQUENCY_K500 	/*!< 500 kbps */
#define SPI_FREQUENCY_M1 				SPI_FREQUENCY_FREQUENCY_M1 		/*!< 1 Mbps */
#define SPI_FREQUENCY_M2 				SPI_FREQUENCY_FREQUENCY_M2 		/*!< 2 Mbps */
#define SPI_FREQUENCY_M4 				SPI_FREQUENCY_FREQUENCY_M4 		/*!< 4 Mbps */
#define SPI_FREQUENCY_M8 				SPI_FREQUENCY_FREQUENCY_M8 		/*!< 8 Mbps */


// =================================== Modes ==============================================
#define SPI_MODE_0						0x00														// CPOL = 0 CPHA = 0
#define SPI_MODE_1						0x04														// CPOL = 0 CPHA = 1
#define SPI_MODE_2						0x02														// CPOL = 1 CPHA = 0
#define SPI_MODE_3						0x06														// CPOL = 1 CPHA = 1

#define SPI_ORDER_MSB_FIRST				0x00														//Most significant bit shifted out first
#define SPI_ORDER_LSB_FIRST				0x01														//Least significant bit shifted out first

#define SPI_PIN_NONE 							(-1)													//jesli nie chcemy uzywac danego pinu


// =================================== Functions ==========================================
spiHandle_t spiInit(NRF_SPI_Type* SPIx, uint8_t SCK, int8_t MOSI, int8_t MISO, uint8_t CS, uint32_t frequency, uint8_t mode, uint8_t order);
uint8_t spiRead(spiHandle_t handle);
void spiSend(spiHandle_t handle, uint8_t byte);

void spiControlChipSelect(spiHandle_t handle, uint8_t state);

#endif /* __SPI__H__ */
