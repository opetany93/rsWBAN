#include "hal.h"
#include "radio.h"
#include "protocol_host.h"

// =======================================================================================
int main(void)
{
	boardInit();
	lcdProtocolInit();
	radioHostInit();
	
	RTC0_Sync();
	RTC1_TimeSlot();
	setFreqCollectData(FREQ_COLLECT_DATA_20Hz);
	startListening();
	
	while(1)
	{
		__WFI();
	}
}
