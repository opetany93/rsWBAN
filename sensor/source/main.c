#include "hal.h"
#include "radio.h"
#include "rtc.h"
#include "protocol.h"
#include "ADXL362.h"
#include "nrf.h"

//=======================================================================================
int main(void)
{
	boardInit();
	Radio* radio = radioSensorInit();
	Rtc* rtc0 = rtcInit(RTC0);
	buttonInterruptInit();
	initProtocol(radio,rtc0);

	while(1)
	{
		sleep();
	}
}


void timeSlotCallback(data_packet_t* dataPacketPtr)
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
		((ADXL362_AXES_t *)(dataPacketPtr->data))->x = (inc += 15);
	}
	else
	{
		inc = 0;
		((ADXL362_AXES_t *)(dataPacketPtr->data))->x = 0;
	}
}
