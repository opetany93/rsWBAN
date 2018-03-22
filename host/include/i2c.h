#ifndef __i2c_H__
#define __i2c_H__

#include <stdint.h>
#include "nrf.h"


// ========================================================================================
#define TWI_ENABLE(TWIx)				TWIx->ENABLE = (TWI_ENABLE_ENABLE_Enabled  << TWI_ENABLE_ENABLE_Pos)
#define TWI_DISABLE(TWIx)				TWIx->ENABLE = (TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos)

// =================================== Frequencies ========================================
#define TWI_FREQUENCY_K100 				TWI_FREQUENCY_FREQUENCY_K100
#define TWI_FREQUENCY_K250 				TWI_FREQUENCY_FREQUENCY_K250
#define TWI_FREQUENCY_K400 				TWI_FREQUENCY_FREQUENCY_K400

/* Read multibyte command */
#define READ_MULTIBYTE_CMD        		0x80UL

//======================================== functions ========================================
void i2cInit(NRF_TWI_Type* NRF_TWIx, uint32_t speed, uint8_t pin_SCL, uint8_t pin_SDA);

void i2cWrite(NRF_TWI_Type* TWIx, uint8_t device_address, uint8_t register_address, uint8_t data);

uint8_t i2cRead(NRF_TWI_Type* TWIx, uint8_t device_address, uint8_t register_address);
void i2cReadMultibyte(NRF_TWI_Type* TWIx, uint8_t device_address, uint8_t register_address, uint8_t* buf, uint8_t length);

#endif /* __i2c_H__ */
