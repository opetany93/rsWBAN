#include "LPS22HB.h"
#include "i2c.h"
#include "mydefinitions.h"

//===================================================================================
void LPS22HB_Init(NRF_TWI_Type* TWIx)
{
	i2cInit(TWI0, TWI_FREQUENCY_K400, SCL_PIN, SDA_PIN);
	i2cWrite(TWI0, LPS22HB_ADR, LPS22HB_CTRL_REG1, 0x5 << 4);
}

//===================================================================================
uint16_t LPS22HB_ReadTemp(NRF_TWI_Type* TWIx)
{
	uint16_t temperature;
	
	i2cReadMultibyte(TWI0, LPS22HB_ADR, LPS22HB_TEMP_OUT_L, (uint8_t*)&temperature, 2);
	
	return temperature;
}

//===================================================================================
uint32_t LPS22HB_ReadPressure(NRF_TWI_Type* TWIx)
{
	uint32_t pressure = 0;
	
	i2cReadMultibyte(TWI0, LPS22HB_ADR, LPS22HB_PRESS_OUT_XL, (uint8_t*)&pressure, 3);
	
	return pressure;
}
