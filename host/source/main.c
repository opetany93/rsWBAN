#include "hal.h"
#include "radio.h"
#include "protocol_host.h"
#include "mydefinitions.h"

// =======================================================================================
int main(void)
{
	boardInit();
	lcdProtocolInit();
	Radio *radio = radioHostInit();
	Protocol *protocol = initProtocol(radio);

	protocol->setFreqCollectData(FREQ_COLLECT_DATA_20Hz);
	protocol->startListening();

	while(1)
	{
		__WFI();
	}
}
