#include "hal.h"
#include "radio.h"
#include "protocol.h"

#include "nrf.h"

extern char packet[PACKET_SIZE];

//=======================================================================================
int main(void)
{
	boardInit();
	Radio *radio = radioSensorInit();
	buttonInterruptInit();
	initProtocol(radio);
	
	while(1)
	{	
		__WFE();						// wait for event, sleep mode
	}
}


void timeSlotCallback()
{
	static uint16_t inc = 0;

	//SPI_ENABLE(SPI0);

	#if FIFO_ENABLED
		ADXL362_ReadFifo(((data_packet_t *)packet)->data, 15);		// 5 samples of 3 axis
	#else
		//ADXL362_ReadXYZ(&((data_packet_t *)packet)->axes);
	#endif

	//SPI_DISABLE(SPI0);

	if( 1500 > inc )
	{
		((data_packet_t *)packet)->axes.x = (inc += 15);
	}
	else
	{
		inc = 0;
		((data_packet_t *)packet)->axes.x = 0;
	}
}
