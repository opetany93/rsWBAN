#include "hal.h"
#include "radio.h"
#include "protocol_host.h"
#include "mydefinitions.h"
#include "uart.h"
#include "rtc.h"
#include "nrf.h"

#include <stdio.h>

extern char uartBuf[100];

// =======================================================================================
int main(void)
{
	boardInit();
	lcdProtocolInit();
	Radio *radio = radioHostInit();
	Rtc* rtc0 = rtcInit(RTC0);
	Rtc* rtc1 = rtcInit(RTC1);

	Protocol *protocol = initProtocol(radio, rtc0, rtc1);

	protocol->setFreqCollectData(FREQ_COLLECT_DATA_20Hz);
	protocol->startListening();

	while(1)
	{
		__WFI();
	}
}


void dataReadyCallback(data_packet_t** packets, uint8_t amountOfConnectedSensors)	// time of execution have to be less than 1,5 ms
{
	if( amountOfConnectedSensors == 1 )
	{
		sprintf(uartBuf, "%d %d %d\r\n", ((ADXL362_AXES_t *)(packets[0]->data))->x,  ((ADXL362_AXES_t *)(packets[0]->data))->y, ((ADXL362_AXES_t *)(packets[0]->data))->z);
		uartWriteS(uartBuf);
	}

//	if( connected_sensors_amount == 1 )
//	{
//		for(uint8_t i = 0; i < 15; i++)
//		{
//			axisTemp = packets[0]->data[i];
//
//			if ( (axisTemp >> 14) == 1 )
//			{
//				axisTemp <<= 2;
//				axisTemp >>= 2;
//
//				axisTemp |= (axisTemp >> 12) << 14;
//
//				axis.y = axisTemp;
//			}
//			else if ( (axisTemp >> 14) == 2 )
//			{
//				axisTemp <<= 2;
//				axisTemp >>= 2;
//
//				axisTemp |= (axisTemp >> 12) << 14;
//
//				axis.z = axisTemp;
//			}
//			else if ( (axisTemp >> 14) == 0 )
//			{
//				axisTemp <<= 2;
//				axisTemp >>= 2;
//
//				axisTemp |= (axisTemp >> 12) << 14;
//
//				axis.x = axisTemp;
//			}
//
//			if ( !((i+1) % 3) )
//			{
//				//uartDmaWriteS(buf, sprintf(buf, "%d %d %d\r\n", axis.x, axis.y, axis.z));
//				sprintf(buf, "%d %d %d\r\n", axis.x, axis.y, axis.z);
//				uartWriteS(buf);
//			}
//		}
//	}

	if( amountOfConnectedSensors == 2 )
	{
		sprintf(uartBuf, "%d %d %d %d\r\n", ((ADXL362_AXES_t *)(packets[0]->data))->x,  ((ADXL362_AXES_t *)(packets[0]->data))->y, ((ADXL362_AXES_t *)(packets[0]->data))->z, ((ADXL362_AXES_t *)(packets[1]->data))->x);
		uartWriteS(uartBuf);
	}

	if( amountOfConnectedSensors == 3 )
	{
		sprintf(uartBuf, "%d %d %d %d %d\r\n", ((ADXL362_AXES_t *)(packets[0]->data))->x,  ((ADXL362_AXES_t *)(packets[0]->data))->y, ((ADXL362_AXES_t *)(packets[0]->data))->z, ((ADXL362_AXES_t *)(packets[1]->data))->x, ((ADXL362_AXES_t *)(packets[2]->data))->x);
		uartWriteS(uartBuf);
	}

	if( amountOfConnectedSensors == 4 )
	{
		sprintf(uartBuf, "%d %d %d %d %d %d\r\n", ((ADXL362_AXES_t *)(packets[0]->data))->x, ((ADXL362_AXES_t *)(packets[0]->data))->y, ((ADXL362_AXES_t *)(packets[0]->data))->z, ((ADXL362_AXES_t *)(packets[1]->data))->x, ((ADXL362_AXES_t *)(packets[2]->data))->x, ((ADXL362_AXES_t *)(packets[3]->data))->x);
		uartWriteS(uartBuf);
	}
}
