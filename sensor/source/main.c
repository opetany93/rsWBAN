#include "hal.h"
#include "radio.h"
#include "protocol.h"

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
