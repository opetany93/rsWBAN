#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>
#include "mydefinitions.h"

//============================== Defines fot RTC ========================================
#define GPIO_TOGGLE_TICK_EVENT     		LED_3				/*!< Pin number to toggle when there is a tick event in RTC */
#define GPIO_TOGGLE_COMPARE0_EVENT 		LED_4				/*!< Pin number to toggle when there is compare event in RTC */
#define GPIO_TOGGLE_COMPARE1_EVENT 		LED_2

#define LFCLK_FREQUENCY           		(32768UL)			/*!< LFCLK frequency in Hertz, constant */
#define RTC_FREQUENCY             		(8UL)				/*!< required RTC working clock RTC_FREQUENCY Hertz. Changable */

#define COMPARE0_COUNTERTIME       		(1UL)				/*!< Get Compare event COMPARE_TIME seconds after the counter starts from 0 */
//#define COMPARE1_COUNTERTIME       	(20UL)

#define COUNTER_PRESCALER         		((LFCLK_FREQUENCY/RTC_FREQUENCY) - 1)  /* f = LFCLK/(prescaler + 1) */

//================================= RTC types ========================================

typedef struct
{
	uint16_t  Year;
	uint8_t   Mon;
	uint8_t   Day;
	uint8_t   Hour;
	uint8_t   Min;
	uint8_t   Sec;
	
}t_rtc;

//=============================== RTC functions ========================================
void rtc_config(void);
void gpio_config(void);


uint32_t structRtcToInt (t_rtc time);
void intRtcToStruct (uint32_t x, t_rtc  * time) ;

void InitRTC (void);
void ReadTimeRTC (t_rtc * rtc);
void SetTimeRTC (void);

#endif /* __RTC__H__ */
