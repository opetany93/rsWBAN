#include "hal.h"
#include "radio.h"
#include "protocol_host.h"
#include "mydefinitions.h"

static void initRtc0()
{
	RTC0->CC[0] = 66;			// rozpoczoczêcie szczelin
	RTC0->CC[1] = 1639;			// wyznaczanie SUF
	
	RTC0->PRESCALER = 0;

	RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
	RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
	RTC0->EVTENSET = RTC_EVTENSET_COMPARE1_Enabled << RTC_EVTENSET_COMPARE1_Pos;
	RTC0->INTENSET = RTC_INTENSET_COMPARE1_Enabled << RTC_INTENSET_COMPARE1_Pos;

	NVIC_SetPriority(RTC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), HIGH_IRQ_PRIO, RTC0_IRQ_PRIORITY));
	NVIC_EnableIRQ(RTC0_IRQn);

	PPI->CH[1].EEP = (uint32_t) &RTC0->EVENTS_COMPARE[1];
	PPI->CH[1].TEP = (uint32_t) &RTC0->TASKS_CLEAR;
	PPI->CHENSET = PPI_CHENSET_CH1_Enabled << PPI_CHENSET_CH1_Pos;

	RTC0->TASKS_CLEAR = 1U;
}

static void initRtc1()
{
	RTC1->PRESCALER = 0;

	RTC1->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
	RTC1->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;

	NVIC_SetPriority(RTC1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), HIGH_IRQ_PRIO, RTC1_IRQ_PRIORITY));
	NVIC_EnableIRQ(RTC1_IRQn);

	RTC1->CC[0] = 393;
	RTC1->TASKS_CLEAR = 1U;
}

// =======================================================================================
int main(void)
{
	boardInit();
	//lcdProtocolInit();
	//radioHostInit();

	initRtc0();
	initRtc1();



	RTC0->TASKS_START = 1U;

	while(1)
	{
		__WFI();
	}
}
