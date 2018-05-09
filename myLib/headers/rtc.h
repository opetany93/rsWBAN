#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>
#include "mydefinitions.h"
#include "nrf.h"



typedef struct{

	NRF_RTC_Type* RTCx;
	void (*clear)(void* rtc);
	void (*start)(void* rtc);
	void (*setPrescaler)(void* rtc, uint16_t presc);
	void (*setCCreg)(void* rtc, uint8_t number, uint32_t value);
	void (*compareEventEnable)(void* rtc, uint8_t nbrOfCompare);
	void (*compareInterruptEnable)(void* rtc, uint8_t nbrOfCompare);
	
}Rtc;

Rtc* rtcInit(NRF_RTC_Type* RTCx);

#endif /* __RTC__H__ */
