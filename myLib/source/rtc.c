#include "rtc.h"

#include "nrf_gpio.h"
#include "mydefinitions.h"
#include "hal.h"
#include "clocks.h"

t_rtc RTC_TimeStruct;
volatile uint32_t RTC_TimeInt32;

const uint8_t DigitTab[]="0123456789";


//====================================== RTC functions ===========================================

void InitRTC (void)
{
	RTC_TimeStruct.Year = 2005;
	RTC_TimeStruct.Mon = 1;
	RTC_TimeStruct.Day = 1;
	RTC_TimeStruct.Hour = 12;
	RTC_TimeStruct.Min = 0;
	RTC_TimeStruct.Sec = 0;


	RTC0 -> TASKS_STOP = 0x1;
	RTC_TimeInt32 = structRtcToInt (RTC_TimeStruct);
	RTC0 -> TASKS_START = 0x1;
}

//===========================================================================================================
uint32_t structRtcToInt (t_rtc time)
{
	uint32_t x;

	x = 365 * (time.Year - 1970);                 // calculate number of days for previous years
	x += (time.Year - 1969) >> 2;                 // add a day for each leap year

	if((time.Mon > 2) && (time.Year % 4 == 0))    // add a day if current year is leap and past Feb 29th
		x++;

	switch(time.Mon)
	{
		case 1: x += 0; break;
		case 2: x += 31; break;
		case 3: x += 59; break;
		case 4: x += 90; break;
		case 5: x += 120; break;
		case 6: x += 151; break;
		case 7: x += 181; break;
		case 8: x += 212; break;
		case 9: x += 243; break;
		case 10: x += 273; break;
		case 11: x += 304; break;
		case 12: x += 334; break;
	}

	x += time.Day - 1;       // finally, add the days into the current month
	x = x * 86400;           //  and calculate the number of seconds in all those days
	x += (time.Hour * 1800);  //  add the number of seconds in the hours
	x += (time.Hour * 1800);  //  add the number of seconds in the hours
	x += (time.Min * 60);    //  ditto the minutes
	x += time.Sec;           // finally, the seconds

	return(x);
}

//===========================================================================================================
// ------ convert binary time to date format ------ 
void intRtcToStruct (uint32_t x, t_rtc  * time) 
{
uint16_t yrs = 99, mon = 99, day = 99, tmp, jday, hrs, min, sec;
uint32_t j, n;

  j = x / 60; /* whole minutes since 1/1/70 */
  sec = x - (60 * j); /* leftover seconds */
  n = j / 60;
  min = j - (60 * n);
  j = n / 24;
  hrs = n - (24 * j);
  j = j + (365 + 366); /* whole days since 1/1/68 */
  day = j / ((4 * 365) + 1);
  tmp = j % ((4 * 365) + 1);
  if(tmp >= (31 + 29)) /* if past 2/29 */
  day++; /* add a leap day */
  yrs = (j - day) / 365; /* whole years since 1968 */
  jday = j - (yrs * 365) - day; /* days since 1/1 of current year */
  if(tmp <= 365 && tmp >= 60) /* if past 2/29 and a leap year then */
  jday++; /* add a leap day */
  yrs += 1968; /* calculate year */

  for(mon = 12; mon > 0; mon--)
  {
    switch (mon)
    {
      case 1: tmp = 0; break;
      case 2: tmp = 31; break;
      case 3: tmp = 59; break;
      case 4: tmp = 90; break;
      case 5: tmp = 120; break;
      case 6: tmp = 151; break;
      case 7: tmp = 181; break;
      case 8: tmp = 212; break;
      case 9: tmp = 243; break;
      case 10: tmp = 273; break;
      case 11: tmp = 304; break;
      case 12: tmp = 334; break;
    }

    if((mon > 2) && !(yrs % 4)) // adjust for leap year 
      tmp++;
    if(jday >= tmp) break;
  }

  day = jday - tmp + 1;   // calculate day in month

  time->Sec = (uint8_t) sec;
  time->Min = (uint8_t) min;
  time->Hour = (uint8_t) hrs;

  time->Day = (uint8_t) day;
  time->Mon = (uint8_t) mon;
  time->Year = yrs;
}

//===========================================================================================================
void SetTimeRTC (void)
{
	uint32_t time;

	time = structRtcToInt (RTC_TimeStruct);
	RTC0->TASKS_STOP = 0x1;
	RTC_TimeInt32 = time;
	RTC0->TASKS_START = 0x1;
}

//===========================================================================================================
void ReadTimeRTC (t_rtc * rtc)
{
	uint32_t time;
  
	RTC0->TASKS_STOP = 0x1;
	time = RTC_TimeInt32;
	RTC0->TASKS_START = 0x1;

	intRtcToStruct (time, rtc);
}

//===========================================================================================================
/** Configures the RTC with TICK for 8Hz, COMPARE0 to 10 sec and COMPARE1 to 20 sec
 */
void rtc_config(void)
{
	startLFCLK();
	
	RTC0->TASKS_STOP = 0;
	
	//RTC config
	NVIC_EnableIRQ(RTC0_IRQn);                                 // Enable Interrupt for the RTC in the core
	NVIC_SetPriority(RTC0_IRQn, 0x02);
	RTC0 -> PRESCALER = COUNTER_PRESCALER;                   // Set prescaler to a TICK of RTC_FREQUENCY
	RTC0 -> CC[0] = COMPARE0_COUNTERTIME * RTC_FREQUENCY;     // Compare0 after approx COMPARE_COUNTERTIME seconds
	//RTC0->CC[1] = COMPARE1_COUNTERTIME * RTC_FREQUENCY;
	
	
	// Enable TICK event and TICK interrupt:
	RTC0 -> EVTENSET = RTC_EVTENSET_TICK_Msk;
	RTC0 -> INTENSET = RTC_INTENSET_TICK_Msk;

	// Enable COMPARE0 event and COMPARE0 interrupt:
	RTC0 -> EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
	RTC0 -> INTENSET = RTC_INTENSET_COMPARE0_Msk;
	
	// Enable COMPARE1 event and COMPARE1 interrupt:
	//RTC0->EVTENSET = RTC_EVTENSET_COMPARE1_Msk;
	//RTC0->INTENSET = RTC_INTENSET_COMPARE1_Msk;
}


//===========================================================================================================
/** Configures PIN8 and PIN9 as outputs
 */
void gpio_config(void)
{
  nrf_gpio_cfg_output(GPIO_TOGGLE_TICK_EVENT);
  nrf_gpio_cfg_output(GPIO_TOGGLE_COMPARE0_EVENT);
	//nrf_gpio_cfg_output(GPIO_TOGGLE_COMPARE1_EVENT);

  nrf_gpio_pin_write(GPIO_TOGGLE_TICK_EVENT, 0);
  nrf_gpio_pin_write(GPIO_TOGGLE_COMPARE0_EVENT, 0);
	//nrf_gpio_pin_write(GPIO_TOGGLE_COMPARE1_EVENT, 0);
}


//===========================================================================================================
/** RTC0 interrupt handler.
 * Triggered on TICK, COMPARE0 and COMPARE1 match.
 */
//void RTC0_IRQHandler()
//{
//  if ((NRF_RTC0->EVENTS_TICK != 0) && ((NRF_RTC0->INTENSET & RTC_INTENSET_TICK_Msk) != 0))
//  {
//    NRF_RTC0 -> EVENTS_TICK = 0;
//		
//    nrf_gpio_pin_toggle(GPIO_TOGGLE_TICK_EVENT);
//  }
//  if ((NRF_RTC0->EVENTS_COMPARE[0] != 0) && ((NRF_RTC0->INTENSET & RTC_INTENSET_COMPARE0_Msk) != 0))
//  {
//    NRF_RTC0 -> EVENTS_COMPARE[0] = 0;
//		RTC_TimeInt32++;
//		NRF_RTC0 -> TASKS_CLEAR = 0x1;
//    nrf_gpio_pin_toggle(GPIO_TOGGLE_COMPARE0_EVENT);
//  }
//	if ((NRF_RTC0->EVENTS_COMPARE[1] != 0) && ((NRF_RTC0->INTENSET & RTC_INTENSET_COMPARE0_Msk) != 0))
//  {
//    NRF_RTC0 -> EVENTS_COMPARE[1] = 0;
//    //nrf_gpio_pin_toggle(GPIO_TOGGLE_COMPARE1_EVENT);
//  }
//}
