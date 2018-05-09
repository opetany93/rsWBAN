#include "rtc.h"

#include <stdlib.h>

static inline void clear(void* rtc)
{
	((Rtc*)rtc)->RTCx->TASKS_CLEAR = 1U;
}

static inline void start(void* rtc)
{
	((Rtc*)rtc)->RTCx->TASKS_START = 1U;
}

static inline void setPrescaler(void* rtc, uint16_t presc)
{
	((Rtc*)rtc)->RTCx->PRESCALER = presc;
}

static inline void setCCreg(void* rtc, uint8_t number, uint32_t value)
{
	((Rtc*)rtc)->RTCx->CC[number] = value;
}

void compareEventEnable(void* rtc, uint8_t nbrOfCompare)
{
	((Rtc*)rtc)->RTCx->EVTENSET = (1 << (RTC_EVTENSET_COMPARE0_Pos + nbrOfCompare));
}

void compareInterruptEnable(void* rtc, uint8_t nbrOfCompare)
{
	((Rtc*)rtc)->RTCx->INTENSET = (1 <<  (RTC_INTENSET_COMPARE0_Pos + nbrOfCompare));
}

Rtc* rtcInit(NRF_RTC_Type* RTCx)
{
	Rtc* rtc = malloc(sizeof(Rtc));

	rtc->RTCx = RTCx;
	rtc->clear = clear;
	rtc->start = start;
	rtc->setPrescaler = setPrescaler;
	rtc->setCCreg = setCCreg;
	rtc->compareEventEnable = compareEventEnable;
	rtc->compareInterruptEnable = compareInterruptEnable;

	return rtc;
}
