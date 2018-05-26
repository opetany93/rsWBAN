#include "rtc.h"

#include <stdlib.h>

#include "nrf_rtc.h"

static inline void clear(void* rtc)
{
	nrf_rtc_task_trigger(((Rtc*)rtc)->RTCx, NRF_RTC_TASK_CLEAR);
}

static inline void start(void* rtc)
{
	nrf_rtc_task_trigger(((Rtc*)rtc)->RTCx, NRF_RTC_TASK_START);
}

static inline void setPrescaler(void* rtc, uint16_t presc)
{
	nrf_rtc_prescaler_set(((Rtc*)rtc)->RTCx, presc);
}

static inline void setCCreg(void* rtc, uint8_t number, uint32_t value)
{
	nrf_rtc_cc_set(((Rtc*)rtc)->RTCx, number, value);
}

void compareEventEnable(void* rtc, rtc_channel_t channel)
{
	nrf_rtc_event_enable(((Rtc*)rtc)->RTCx, (1 << (RTC_EVTENSET_COMPARE0_Pos + channel)));
}

void compareInterruptEnable(void* rtc, rtc_channel_t channel)
{
	nrf_rtc_int_enable(((Rtc*)rtc)->RTCx, (1 << (RTC_EVTENSET_COMPARE0_Pos + channel)));
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
