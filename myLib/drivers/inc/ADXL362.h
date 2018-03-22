#ifndef __ADXL362_H__
#define __ADXL362_H__

#include <stdint.h>

#define FIFO_ENABLED 0

// ======================================== ADXL362 registers ==============================
#define ADXL362_DEVID_AD          	0x00    // read only
#define ADXL362_DEVID_MST			0x01		// read only
#define ADXL362_PARTID				0x02		// read only
#define ADXL362_REVID           	0x03		// read only
#define ADXL362_XDATA            	0x08		// read only
#define ADXL362_YDATA             	0x09		// read only
#define ADXL362_ZDATA          		0x0A		// read only
#define ADXL362_STATUS          	0x0B		// read only
#define ADXL362_FIFO_ENTRIES_L		0x0C		// read only
#define ADXL362_FIFO_ENTRIES_H		0x0D		// read only
#define ADXL362_XDATA_L				0x0E		// read only
#define ADXL362_XDATA_H				0x0F		// read only
#define ADXL362_YDATA_L				0x10		// read only
#define ADXL362_YDATA_H				0x11		// read only
#define ADXL362_ZDATA_L				0x12		// read only
#define ADXL362_ZDATA_H				0x13		// read only
#define ADXL362_TEMP_L				0x14		// read only
#define ADXL362_TEMP_H				0x15		// read only
#define ADXL362_SOFT_RESET			0x1F
#define ADXL362_THRESH_ACT_L		0x20
#define ADXL362_THRESH_ACT_H		0x21
#define ADXL362_TIME_ACT			0x22
#define ADXL362_THRESH_INACT_L		0x23
#define ADXL362_THRESH_INACT_H		0x24
#define ADXL362_TIME_INACT_H		0x25
#define ADXL362_TIM_INACT_L			0x26
#define ADXL362_ACT_INACT_CTL		0x27
#define ADXL362_FIFO_CONTROL		0x28
#define ADXL362_FIFO_SAMPLES		0x29
#define ADXL362_INTMAP1				0x2A
#define ADXL362_INTMAP2				0x2B
#define ADXL362_FILTER_CTL			0x2C
#define ADXL362_POWER_CTL			0x2D
#define ADXL362_SELF_TEST			0x2E

// Register specific bits
// Device ID Register 0x00 - Read value should be 0xAD

// Device ID Register 0x01 - Read value should be 0x1D

// Part ID Register 0x02 - Read value should be 0xF2

// Silicon Revision ID Register 0x03 - Read value should be 0x02

// X-AXIS Data Register 0x08 - Used to read 8 MSB in Power conscious applications

// Y-AXIS Data Register 0x09 - Used to read 8 MSB in Power conscious applications

// Z-AXIS Data Register 0x0A - Used to read 8 MSB in Power conscious applications

// Status Register 0x0B
#define ADXL362_ERR_USER_REGS			7
#define ADXL362_AWAKE					6
#define ADXL362_INACT					5
#define ADXL362_ACT						4
#define ADXL362_FIFO_OVERRUN			3
#define ADXL362_FIFO_WATERMARK			2
#define ADXL362_FIFO_READY				1
#define ADXL362_DATA_READY				0

// FIFO Entries L Register 0x0C - Contains number of FIFO Entries in FIFO Buffer (0 to 512)
// FIFO Entries H Register 0x0D [15:10] are unused

// X-AXIS Data Register L and H 0x0E and 0x0F

// Y-AXIS Data Register L and H 0x10 and 0x11

// Z-AXIS Data Register L and H 0x12 and 0x13

// Temperature Register L and H 0x14 and 0x15

// Software Reset Register
#define ADXL362_RESET_CMD				0x52

// Activity Threshold Register L and H 0x20 and 0x21

// Activity Timer Register 0x22

// Inactivity Threshold Register L and H 0x23 and 0x24

// Inactivity Timer Register L and H 0x25 and 0x26

// Activity / Inactivity Control Register 0x27
#define ADXL362_LINKLOOP				4				// [5:4]
#define ADXL362_INACT_REF				3
#define ADXL362_INACT_EN				2
#define ADXL362_ACT_REF					1
#define ADXL362_ACT_EN					0

// FIFO Control Register 0x28
#define ADXL362_AH						3
#define ADXL362_FIFO_TEMP				2
#define ADXL362_FIFO_MODE				0				// [1:0]

// FIFO Modes
#define ADXL362_FIFO_OLDEST_SAVED_MODE	0x01
#define ADXL362_FIFO_STREAM_MODE		0x02
#define ADXL362_FIFO_TRIGGERED_MODE		0x03

// FIFO Samples Register 0x29

// INT1/INT2 Function Map Registers 0x2A and 0x2B
#define ADXL362_INT_LOW					7
#define ADXL362_AWAKE					6
#define ADXL362_INACT					5
#define ADXL362_ACT						4
#define ADXL362_FIFO_OVERRUN			3
#define ADXL362_FIFO_WATERMARK			2
#define ADXL362_FIFO_READY				1
#define ADXL362_DATA_READY				0

// Filter Control Register 0x2C
#define ADXL362_RANGE_8G				7
#define ADXL362_RANGE_4G				6
#define ADXL362_HALF_BW					4
#define ADXL362_EXT_SAMPLE				3	
#define ADXL362_ODR						0				// [2:0]

// Power Control Register 0x2D
#define ADXL362_EXT_CLK					6
#define ADXL362_LOW_NOISE				4				// 4 - low noise mode, 5 - ultralow noise mode
#define ADXL362_WAKEUP					3
#define ADXL362_AUTOSLEEP				2
#define ADXL362_MEASURE					1

// ===============================================================================================
typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
}ADXL362_AXES_t;



//======================================== functions ========================================
int8_t    ADXL362_Init(void (*spiSendFunc)(uint8_t byte), uint8_t (*spiReadFunc)(void), void (*spiControlChipSelect)(uint8_t state));
double	  ADXL362_ReadTemp(void);
int16_t   ADXL362_ReadX(void);
int16_t   ADXL362_ReadY(void);
int16_t   ADXL362_ReadZ(void);
int8_t    ADXL362_ReadXYZ(ADXL362_AXES_t *axes);

int8_t 	  ADXL362_FifoInit(uint16_t samplesInFifo);
int8_t 	  ADXL362_ReadFifo(uint16_t* buffer, uint8_t length);
#endif /* __ADXL362_H__ */
