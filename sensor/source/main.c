#include "hal.h"
#include "radio.h"

//=======================================================================================
int main(void)
{
	boardInit();
	radioSensorInit();
	buttonInterruptInit();
	
	while(1)
	{	
		__WFE();						// wait for event, sleep mode
	}
}
